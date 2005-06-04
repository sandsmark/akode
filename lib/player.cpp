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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <pthread.h>
#include <semaphore.h>

#include "audioframe.h"
#include "audiobuffer.h"
#include "streamdecoder.h"
#include "framedecoder.h"
#include "frametostream_decoder.h"
#include "streamtoframe_decoder.h"
#include "frametostream_sink.h"
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
                   , buffer(0)
                   , frame_decoder(0)
                   , decoder(0)
                   , in_decoder(0)
                   , resampler(0)
                   , converter(0)
                   , volume_filter(0)
                   , sink(0)
                   , manager(0)
                   , sample_rate(0)
                   , state(Closed)
                   , halt(false)
                   , pause(false)
                   , running(false)
                   {};

    File *src;
    AudioBuffer *buffer;

    FrameDecoder *frame_decoder;
    StreamDecoder *decoder;
    StreamToFrameDecoder *in_decoder;
    Resampler *resampler;
    Converter *converter;
    // Volatile because it can be created and destroyed during playback
    VolumeFilter* volatile volume_filter;
    Sink *sink;
    Player::Manager *manager;

    SinkPluginHandler sink_handler;
    DecoderPluginHandler decoder_handler;
    ResamplerPluginHandler resampler_handler;

    unsigned int sample_rate;
    State state;

    volatile bool halt;
    volatile bool pause;
    bool running;
    pthread_t player_thread;
    sem_t pause_sem;

    void setState(Player::State s) {
        state = s;
        if (manager) manager->stateChangeEvent(s);
    }
};

// The player-thread. It is controlled through the two variables
// halt and seek_pos in m_data
static void* run_player(void* arg) {
    Player::private_data *m_data = (Player::private_data*)arg;

    AudioFrame frame;
    AudioFrame re_frame;
    AudioFrame c_frame;
    bool no_error = true;
    m_data->halt = false;

    while(true) {
        if (m_data->pause) sem_wait(&m_data->pause_sem);
        if (m_data->halt) break;

        no_error = m_data->in_decoder->readFrame(&frame);

        if (!no_error) {
            if (m_data->in_decoder->eof())
                goto eof;
            else
            if (m_data->in_decoder->error())
                goto error;
            else
                AKODE_DEBUG("Blip?");
        }
        else {
            AudioFrame* out_frame = &frame;
            if (m_data->resampler) {
                m_data->resampler->doFrame(out_frame, &re_frame);
                out_frame = &re_frame;
            }

            if (m_data->converter) {
                m_data->converter->doFrame(out_frame, &c_frame);
                out_frame = &c_frame;
            }

            if (m_data->volume_filter)
                m_data->volume_filter->doFrame(out_frame);

            no_error = m_data->sink->writeFrame(out_frame);

            if (!no_error) {
                // ### Check type of error
                goto error;
            }
        }
    }

    m_data->decoder->halt();
    m_data->running = false;
    return (void*)0;

error:
    m_data->running = false;
    m_data->decoder->halt();
    if (m_data->manager)
        m_data->manager->errorEvent();
    return (void*)0;

eof:
    m_data->running = false;
    m_data->decoder->halt();
    if (m_data->manager)
        m_data->manager->eofEvent();
    return (void*)0;
}

Player::Player() {
    m_data = new private_data;
    sem_init(&m_data->pause_sem,0,0);
}

Player::~Player() {
    close();
    sem_destroy(&m_data->pause_sem);
    delete m_data;
}

bool Player::open(string sinkname) {
    if (state() != Closed)
        close();

    m_data->sink_handler.load(sinkname);
    if (!m_data->sink_handler.isLoaded()) {
        AKODE_DEBUG("Could not load " << sinkname << "-sink");
        return false;
    }
    m_data->sink = m_data->sink_handler.openSink();
    if (!m_data->sink->open()) {
        AKODE_DEBUG("Could not open " << sinkname << "-sink");
        return false;
    }
    setState(Open);
    return true;
}

void Player::close() {
    if (state() == Closed) return;
    if (state() != Open)
        unload();

    delete m_data->volume_filter;
    m_data->volume_filter = 0;

    delete m_data->sink;
    m_data->sink = 0;
    m_data->sink_handler.unload();
    setState(Closed);
}

bool Player::load(string filename) {
    if (state() == Closed) return false;

    if (state() == Paused || state() == Playing)
        stop();

    if (state() == Loaded)
        unload();

//    m_data->src = new MMapFile(filename.c_str());
    m_data->src = new LocalFile(filename.c_str());

    string format = Magic::detectFile(m_data->src);
    if (!format.empty())
    	AKODE_DEBUG("Guessed format: " << format)
    else {
    	AKODE_DEBUG("akode: Cannot detect mimetype");
        delete m_data->src;
        m_data->src = 0;
        return false;
    }

    if (!m_data->decoder_handler.load(format)) {
    	AKODE_DEBUG("akode: Could not load " << format << "-decoder");
        delete m_data->src;
        m_data->src = 0;
        return false;
    }

    m_data->frame_decoder = m_data->decoder_handler.openFrameDecoder(m_data->src);
    if (!m_data->frame_decoder) {
        AKODE_DEBUG("Failed to open FrameDecoder");
        m_data->decoder_handler.unload();
        delete m_data->src;
        m_data->src = 0;
        return false;
    }

    AudioFrame first_frame;

    if (!m_data->frame_decoder->readFrame(&first_frame)) {
        AKODE_DEBUG("Failed to decode first frame");
        delete m_data->frame_decoder;
        m_data->frame_decoder = 0;
        m_data->decoder_handler.unload();
        delete m_data->src;
        m_data->src = 0;
        return false;
    }

    int state = m_data->sink->setAudioConfiguration(&first_frame);
    if (state < 0) {
        AKODE_DEBUG("The sink could not be configured for this format");
        delete m_data->frame_decoder;
        m_data->frame_decoder = 0;
        m_data->decoder_handler.unload();
        delete m_data->src;
        m_data->src = 0;
        return false;
    }
    else
    if (state > 0) {
        // Configuration not 100% accurate
        m_data->sample_rate = m_data->sink->audioConfiguration()->sample_rate;
        if (m_data->sample_rate != first_frame.sample_rate) {
            if (!m_data->resampler) {
                m_data->resampler_handler.load("fast");
                m_data->resampler = m_data->resampler_handler.openResampler();
            }
            m_data->resampler->setSampleRate(m_data->sample_rate);
        }
        int out_channels = m_data->sink->audioConfiguration()->channels;
        int in_channels = first_frame.channels;
        if (in_channels != out_channels) {
            // ### We don't do mixing yet
            AKODE_DEBUG("Sample has wrong number of channels");
            delete m_data->frame_decoder;
            m_data->frame_decoder = 0;
            m_data->decoder_handler.unload();
            delete m_data->src;
            m_data->src = 0;
            return false;
        }
        int out_width = m_data->sink->audioConfiguration()->sample_width;
        int in_width = first_frame.sample_width;
        if (in_width != out_width) {
            if (!m_data->converter)
                m_data->converter = new Converter(out_width);
            else
                m_data->converter->setSampleWidth(out_width);
        }
    }
    else
    {
        delete m_data->resampler;
        delete m_data->converter;
        m_data->resampler = 0;
        m_data->converter = 0;
    }

    delete m_data->buffer;
    m_data->buffer = new AudioBuffer(16);
    m_data->buffer->put(&first_frame, false);

    setState(Loaded);

    return true;
}

void Player::unload() {
    if (state() == Closed || state() == Open) return;
    if (state() == Paused || state() == Playing)
        stop();

    delete m_data->frame_decoder;
    delete m_data->src;

    m_data->frame_decoder = 0;
    m_data->src = 0;
    m_data->decoder_handler.unload();

    setState(Open);
}

void Player::play() {
    if (state() == Closed || state() == Open) return;
    if (state() == Playing) return;

    if (state() == Paused) {
        return resume();
    }

    if (!m_data->buffer) { // stop() has been called
        m_data->buffer = new AudioBuffer(16);
        m_data->frame_decoder->seek(0);
    }

    // connect the streams to play
    m_data->decoder = new FrameToStreamDecoder(m_data->frame_decoder);
    m_data->in_decoder = new StreamToFrameDecoder(m_data->decoder, m_data->buffer);
    m_data->in_decoder->setBlocking(true);

    //m_data->player_thread = new pthread_t;
    if (pthread_create(&m_data->player_thread, 0, run_player, m_data) == 0) {
        m_data->running = true;
        setState(Playing);
    } else {
        m_data->running = false;
        delete m_data->in_decoder;
        delete m_data->decoder;
        delete m_data->buffer;

        m_data->in_decoder = 0;
        m_data->decoder    = 0;
        m_data->buffer     = 0;
        setState(Loaded);
    }
}

void Player::stop() {
    if (state() == Closed || state() == Open) return;
    if (state() == Loaded) return;

    // Needs to set halt first to avoid the paused thread to play a soundbite
    m_data->halt = true;
    if (state() == Paused) resume();

    if (m_data->running) {
        m_data->buffer->release();
        pthread_join(m_data->player_thread, 0);
        m_data->running = false;
    }

    delete m_data->in_decoder;
    delete m_data->decoder;
    delete m_data->buffer;

    m_data->in_decoder = 0;
    m_data->decoder    = 0;
    m_data->buffer     = 0;

    setState(Loaded);
}

void Player::pause() {
    if (state() == Closed || state() == Open || state() == Loaded) return;
    if (state() == Paused) return;

    //m_data->buffer->pause();
    m_data->pause = true;
    setState(Paused);
}

void Player::resume() {
    if (state() != Paused) return;

    m_data->pause = false;
    sem_post(&m_data->pause_sem);

    setState(Playing);
}


void Player::setVolume(float f) {
    if (state() == Closed) return;
    if (f < 0.0 || f > 1.0) return;

    if (f != 1.0 && !m_data->volume_filter) {
        VolumeFilter *vf = new VolumeFilter();
        vf->setVolume(f);
        m_data->volume_filter = vf;
    }
    else if (f != 1.0) {
        m_data->volume_filter->setVolume(f);
    }
    else if (m_data->volume_filter) {
        VolumeFilter *f = m_data->volume_filter;
        m_data->volume_filter = 0;
        delete f;
    }
}

float Player::volume() const {
    if (!m_data->volume_filter) return 1.0;
    else
        return m_data->volume_filter->volume();
}

File* Player::file() const {
    return m_data->src;
}

Sink* Player::sink() const {
    return m_data->sink;
}

Decoder* Player::decoder() const {
    return m_data->in_decoder;
}

Resampler* Player::resampler() const {
    return m_data->resampler;
}

Player::State Player::state() const {
    return m_data->state;
}

void Player::setManager(Manager *manager) {
    m_data->manager = manager;
}

void Player::setState(Player::State state) {
    m_data->setState(state);
}

} // namespace
