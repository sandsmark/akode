/*  aKode: Player

    Copyright (C) 2004-2005 Allan Sandfeld Jensen <kde@carewolf.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <pthread.h>
#include <semaphore.h>
#include <assert.h>

#include "audioframe.h"
#include "audiobuffer.h"
#include "decoder.h"
#include "buffered_decoder.h"
#include "mmapfile.h"
#include "localfile.h"
#include "volumefilter.h"

#include "sink.h"
#include "converter.h"
#include "resampler.h"
#include "magic.h"

#include "player.h"

#ifndef NDEBUG
#include <iostream>
#define AKODE_DEBUG(x) {std::cerr << "akode: " << x << "\n";}
#else
#define AKODE_DEBUG(x) { }
#endif

namespace aKode {

struct Player::private_data
{
    private_data() : src(0)
                   , frame_decoder(0)
                   , resampler(0)
                   , converter(0)
                   , volume_filter(0)
                   , sink(0)
                   , manager(0)
                   , monitor(0)
                   , decoder_plugin(0)
                   , resampler_plugin(0)
                   , sample_rate(0)
                   , state(Closed)
                   , my_file(false)
                   , start_pos(0)
                   , halt(false)
                   , pause(false)
                   , detached(false)
                   , running(false)
                   {};

    File *src;

    Decoder *frame_decoder;
    BufferedDecoder buffered_decoder;
    Resampler *resampler;
    Converter *converter;
    // Volatile because it can be created and destroyed during playback
    VolumeFilter* volatile volume_filter;
    Sink *sink;
    Player::Manager *manager;
    Player::Monitor *monitor;

    const char* decoder_plugin;
    const char* resampler_plugin;

    SinkPluginHandler sink_handler;
    DecoderPluginHandler decoder_handler;
    ResamplerPluginHandler resampler_handler;

    unsigned int sample_rate;
    State state;
    bool my_file;
    int start_pos;

    volatile bool halt;
    volatile bool pause;
    volatile bool detached;
    bool running;
    pthread_t player_thread;
    sem_t pause_sem;

    void setState(Player::State s) {
        state = s;
        if (manager) manager->stateChangeEvent(s);
    }
    // Called for detached players
    void cleanup() {
        buffered_decoder.stop();
        buffered_decoder.closeDecoder();

        delete frame_decoder;
        if (my_file)
            delete src;

        frame_decoder = 0;
        src = 0;
        decoder_handler.unload();

        delete resampler;
        delete converter;
        resampler = 0;
        converter = 0;

        delete this;
    }
};

// The player-thread. It is controlled through the two variables
// halt and seek_pos in d
static void* run_player(void* arg) {
    Player::private_data *d = (Player::private_data*)arg;

    AudioFrame frame;
    AudioFrame re_frame;
    AudioFrame c_frame;
    bool no_error = true;
    d->halt = false;

    while(true) {
        if (d->pause) {
            d->sink->pause();
            sem_wait(&d->pause_sem);
            d->sink->resume();
        }
        if (d->halt) break;

        no_error = d->buffered_decoder.readFrame(&frame);

        if (!no_error) {
            if (d->buffered_decoder.eof())
                goto eof;
            else
            if (d->buffered_decoder.error())
                goto error;
            else
                AKODE_DEBUG("Blip?");
        }
        else {
            AudioFrame* out_frame = &frame;
            if (d->resampler) {
                d->resampler->doFrame(out_frame, &re_frame);
                out_frame = &re_frame;
            }

            if (d->converter) {
                d->converter->doFrame(out_frame, &c_frame);
                out_frame = &c_frame;
            }

            if (d->volume_filter)
                d->volume_filter->doFrame(out_frame);

            no_error = d->sink->writeFrame(out_frame);

            if (d->monitor)
                d->monitor->writeFrame(out_frame);

            if (!no_error) {
                // ### Check type of error
                goto error;
            }
        }
    }

// Stoped by Player::stop()
//     d->buffered_decoder->stop();
    assert(!d->detached);
    return (void*)0;

error:
    if (d->detached) d->cleanup();
    else {
        d->buffered_decoder.stop();
        if (d->manager)
            d->manager->errorEvent();
    }
    return (void*)0;

eof:
    if (d->detached) d->cleanup();
    else {
        d->buffered_decoder.stop();
        if (d->manager)
            d->manager->eofEvent();
    }
    return (void*)0;
}

Player::Player() {
    d = new private_data;
    sem_init(&d->pause_sem,0,0);
}

Player::~Player() {
    close();
    sem_destroy(&d->pause_sem);
    delete d;
}

bool Player::open(const char* sinkname) {
    if (state() != Closed)
        close();

    assert(state() == Closed);

    d->sink_handler.load(sinkname);
    if (!d->sink_handler.isLoaded()) {
        AKODE_DEBUG("Could not load " << sinkname << "-sink");
        return false;
    }
    d->sink = d->sink_handler.openSink();
    if (!d->sink) {
        AKODE_DEBUG("Could not create " << sinkname << "-sink");
        return false;
    }
    if (!d->sink->open()) {
        AKODE_DEBUG("Could not open " << sinkname << "-sink");
        delete d->sink;
        d->sink = 0;
        return false;
    }
    setState(Open);
    return true;
}

void Player::close() {
    if (state() == Closed) return;
    if (state() != Open)
        unload();

    assert(state() == Open);

    delete d->volume_filter;
    d->volume_filter = 0;

    delete d->sink;
    d->sink = 0;
    d->sink_handler.unload();
    setState(Closed);
}

bool Player::load(const char* filename) {
    if (state() == Closed) return false;

    if (state() == Paused || state() == Playing)
        stop();

    if (state() == Loaded)
        unload();

    assert(state() == Open);

    d->src = new MMapFile(filename);
    // Test if the file _can_ be mmaped
    if (!d->src->openRO()) {
        delete d->src;
        d->src = new LocalFile(filename);
        if (!d->src->openRO()) {
            AKODE_DEBUG("Could not open " << filename);
            delete d->src;
            d->src = 0;
            return false;
        }
    }
    // Some of the later code expects it to be closed
    d->src->close();
    d->my_file = true;

    return load();
}

bool Player::load(File *file) {
    if (state() == Closed) return false;

    if (state() == Paused || state() == Playing)
        stop();

    if (state() == Loaded)
        unload();

    assert(state() == Open);

    if (!file->openRO())
        return false;
    file->close();

    d->src = file;
    d->my_file = false;

    return load();
}

bool Player::load() {
    if (d->decoder_plugin) {
        if (!d->decoder_handler.load(d->decoder_plugin))
            AKODE_DEBUG("Could not load " << d->decoder_plugin << "-decoder");
    }

    if (!d->decoder_handler.isLoaded()) {
        string format = Magic::detectFile(d->src);
        if (!format.empty())
            AKODE_DEBUG("Guessed format: " << format)
        else {
            AKODE_DEBUG("Cannot detect mimetype");
            delete d->src;
            d->src = 0;
            return false;
        }
        if (!d->decoder_handler.load(format))
            AKODE_DEBUG("Could not load " << format << "-decoder");
    }

    if (!d->decoder_handler.isLoaded()) {
        delete d->src;
        d->src = 0;
        return false;
    }

    d->frame_decoder = d->decoder_handler.openDecoder(d->src);
    if (!d->frame_decoder) {
        AKODE_DEBUG("Failed to open Decoder");
        d->decoder_handler.unload();
        delete d->src;
        d->src = 0;
        return false;
    }

    AudioFrame first_frame;

    if (!d->frame_decoder->readFrame(&first_frame)) {
        AKODE_DEBUG("Failed to decode first frame");
        delete d->frame_decoder;
        d->frame_decoder = 0;
        d->decoder_handler.unload();
        delete d->src;
        d->src = 0;
        return false;
    }

    int state = d->sink->setAudioConfiguration(&first_frame);
    if (state < 0) {
        AKODE_DEBUG("The sink could not be configured for this format");
        delete d->frame_decoder;
        d->frame_decoder = 0;
        d->decoder_handler.unload();
        delete d->src;
        d->src = 0;
        return false;
    }
    else
    if (state > 0) {
        // Configuration not 100% accurate
        d->sample_rate = d->sink->audioConfiguration()->sample_rate;
        if (d->sample_rate != first_frame.sample_rate) {
            AKODE_DEBUG("Resampling to " << d->sample_rate);
            if (!d->resampler) {
                if (d->resampler_plugin) {
                    d->resampler_handler.load(d->resampler_plugin);
                    d->resampler = d->resampler_handler.openResampler();
                }
                if (!d->resampler) {
                    d->resampler_handler.load("fast");
                    d->resampler = d->resampler_handler.openResampler();
                }
            }
            d->resampler->setSampleRate(d->sample_rate);
        }
        int out_channels = d->sink->audioConfiguration()->channels;
        int in_channels = first_frame.channels;
        if (in_channels != out_channels) {
            // ### We don't do mixing yet
            AKODE_DEBUG("Sample has wrong number of channels");
            delete d->frame_decoder;
            d->frame_decoder = 0;
            d->decoder_handler.unload();
            delete d->src;
            d->src = 0;
            return false;
        }
        int out_width = d->sink->audioConfiguration()->sample_width;
        int in_width = first_frame.sample_width;
        if (in_width != out_width) {
            AKODE_DEBUG("Converting to " << out_width << "bits");
            if (!d->converter)
                d->converter = new Converter(out_width);
            else
                d->converter->setSampleWidth(out_width);
        }
    }
    else
    {
        delete d->resampler;
        delete d->converter;
        d->resampler = 0;
        d->converter = 0;
    }

    // connect the streams to play
    d->buffered_decoder.setBlockingRead(true);
    d->buffered_decoder.openDecoder(d->frame_decoder);
    d->buffered_decoder.buffer()->put(&first_frame);

    setState(Loaded);

    return true;
}

void Player::unload() {
    if (state() == Closed || state() == Open) return;
    if (state() == Paused || state() == Playing)
        stop();

    assert(state() == Loaded);

    delete d->frame_decoder;
    if (d->my_file)
        delete d->src;

    d->frame_decoder = 0;
    d->src = 0;
    d->decoder_handler.unload();

    delete d->resampler;
    delete d->converter;
    d->resampler = 0;
    d->converter = 0;

    setState(Open);
}

void Player::play() {
    if (state() == Closed || state() == Open) return;
    if (state() == Playing) return;

    if (state() == Paused) {
        return resume();
    }

    assert(state() == Loaded);

    d->frame_decoder->seek(0);

    // Start buffering
    d->buffered_decoder.start();

    if (pthread_create(&d->player_thread, 0, run_player, d) == 0) {
        d->running = true;
        setState(Playing);
    } else {
        d->running = false;
        setState(Loaded);
    }
}

void Player::detach() {
    if (state() == Closed || state() == Open) return;
    if (state() == Loaded) return;

    if (state() == Paused) resume();

    assert(state() == Playing);

    if (d->running) {
        pthread_detach(d->player_thread);
        d->running = false;
    }

    private_data * d_new  = new private_data;
    d_new->sink = d->sink;
    d_new->volume_filter = d->volume_filter;
    d_new->sample_rate = d->sample_rate;
    d_new->state = d->state;

    d->detached = true;
    d = d_new;

    setState(Open);
}


void Player::stop() {
    if (state() == Closed || state() == Open) return;
    if (state() == Loaded) return;

    // Needs to set halt first to avoid the paused thread playing a soundbite
    d->halt = true;
    if (state() == Paused) resume();

    assert(state() == Playing);

    d->buffered_decoder.stop();

    if (d->running) {
        pthread_join(d->player_thread, 0);
        d->running = false;
    }

    d->buffered_decoder.closeDecoder();

    setState(Loaded);
}

// Much like stop except we don't send a halt signal
void Player::wait() {
    if (state() == Closed || state() == Open) return;
    if (state() == Loaded) return;

    if (state() == Paused) resume();

    assert(state() == Playing);

    if (d->running) {
        pthread_join(d->player_thread, 0);
        d->running = false;
    }

    d->buffered_decoder.closeDecoder();

    setState(Loaded);
}

void Player::pause() {
    if (state() == Closed || state() == Open || state() == Loaded) return;
    if (state() == Paused) return;

    assert(state() == Playing);

    //d->buffer->pause();
    d->pause = true;
    setState(Paused);
}

void Player::resume() {
    if (state() != Paused) return;

    d->pause = false;
    sem_post(&d->pause_sem);

    setState(Playing);
}


void Player::setVolume(float f) {
    if (f < 0.0 || f > 1.0) return;

    if (f != 1.0 && !d->volume_filter) {
        VolumeFilter *vf = new VolumeFilter();
        vf->setVolume(f);
        d->volume_filter = vf;
    }
    else if (f != 1.0) {
        d->volume_filter->setVolume(f);
    }
    else if (d->volume_filter) {
        VolumeFilter *f = d->volume_filter;
        d->volume_filter = 0;
        delete f;
    }
}

float Player::volume() const {
    if (!d->volume_filter) return 1.0;
    else
        return d->volume_filter->volume();
}

File* Player::file() const {
    return d->src;
}

Sink* Player::sink() const {
    return d->sink;
}

Decoder* Player::decoder() const {
    return &d->buffered_decoder;
}

Resampler* Player::resampler() const {
    return d->resampler;
}

Player::State Player::state() const {
    return d->state;
}

void Player::setDecoderPlugin(const char* plugin) {
    if (plugin && strncmp(plugin, "auto", 4) == 0) plugin = 0;
    d->decoder_plugin = plugin;
}

void Player::setResamplerPlugin(const char* plugin) {
    if (plugin && strncmp(plugin, "fast", 4) == 0) plugin = 0;
    d->resampler_plugin = plugin;
}

void Player::setManager(Manager *manager) {
    d->manager = manager;
}

void Player::setMonitor(Monitor *monitor) {
    d->monitor = monitor;
}

void Player::setState(Player::State state) {
    d->setState(state);
}

} // namespace
