/*  aKode: Buffered Decoder

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
#include <assert.h>

#include "audioframe.h"
#include "audiobuffer.h"
#include "decoder.h"
#include "crossfader.h"
#include "buffered_decoder.h"

namespace aKode {

// States are used to ensure we always have a welldefined state
enum BufferedDecoderStatus { Closed, Open, Playing, Paused, XFadingSeek };

struct BufferedDecoder::private_data
{
    private_data() : buffer(0)
                   , decoder(0)
                   , xfader(0)
                   , fading_time(50)
                   , buffer_size(16)
                   , blocking(false)
                   , running(false)
                   , state(Closed)
                   , halt(false)
                   , done(true)
                   , seek_pos(-1) {};
    AudioBuffer *buffer;
    Decoder *decoder;
    CrossFader *xfader;
    unsigned int fading_time, buffer_size;
    bool blocking;
    bool running;
    BufferedDecoderStatus state;

    // Thread controls
    volatile bool halt, done;
    volatile long seek_pos;
    pthread_t thread;
};

// The decoder-thread. It is controlled through the two variables
// halt and seek_pos in d
static void* run_decoder(void* arg) {
    BufferedDecoder::private_data *d = (BufferedDecoder::private_data*)arg;

    AudioFrame frame;
    bool no_error;

    while(true) {
        if (d->halt) break;
        if (d->seek_pos>=0) {
            d->decoder->seek(d->seek_pos);
            d->seek_pos = -1;
        }

        no_error = d->decoder->readFrame(&frame);
        if (no_error)
            d->buffer->put(&frame, true);
        else {
            if (d->decoder->error() || d->decoder->eof()) {
                break;
            }
        }
    }

    d->done = true;
    d->buffer->setEOF();

    return (void*)0;
}

BufferedDecoder::BufferedDecoder(){
    d = new private_data;
}

BufferedDecoder::~BufferedDecoder() {
    if (d->state != Closed) closeDecoder();
    delete d;
}

void BufferedDecoder::openDecoder(Decoder *decoder) {
    if (d->state != Closed) closeDecoder();

    d->decoder = decoder;
    d->buffer = new AudioBuffer(d->buffer_size);
    d->state = Open;
}

void BufferedDecoder::closeDecoder() {
    if (d->state == Closed) return;
    if (d->state != Open) stop();

    delete d->buffer;
    d->buffer = 0;
    d->decoder = 0;
    d->state = Closed;
}

void BufferedDecoder::start()
{
    if (d->state != Open) return;

    d->halt = false;
    d->done = false;
    d->seek_pos = -1;

    d->buffer->reset();

    if (pthread_create(&d->thread, 0, run_decoder, d) == 0) {
        d->running = true;
    }

    d->state = Playing;
}

void BufferedDecoder::stop() {
    if (d->state == Closed || d->state == Open) return;

    if (d->state != Playing) {
        // Stop fading
        delete d->xfader;
        d->xfader = 0;
    }

    d->buffer->release();

    if (d->running) {
        d->halt = true;
        pthread_join(d->thread, 0);
        d->running = false;
        assert(d->done);
    }

    d->state = Open;
}

bool BufferedDecoder::readFrame(AudioFrame* frame)
{
    if (d->state == Closed || eof()) return false;
    if (d->state == Open) start();

    // Potentially blocking..
    if (d->buffer->get(frame, d->blocking)) {
        if (d->state == XFadingSeek) {
            if(!d->xfader->doFrame(frame)) {
                delete d->xfader;
                d->xfader = 0;
                d->state = Playing;
            }
        }
        return true;
    }
    else
        return false;
}

long BufferedDecoder::length() {
    if (d->decoder)
        return d->decoder->length();
    else
        return -1;
}

long BufferedDecoder::position() {
    long pos = -1;
    pos = d->seek_pos;
    if (pos > 0) return pos;
    if (d->buffer) {
        pos = d->buffer->position();
        if (pos > 0) return pos;
    }
    if (d->decoder) {
        pos = d->decoder->position();
    }
    return pos;
}

bool BufferedDecoder::eof() {
    return d->done || (d->buffer && d->buffer->eof());
}

bool BufferedDecoder::error() {
    return d->decoder && d->decoder->error();
}

bool BufferedDecoder::seekable() {
    if (d->decoder)
        return d->decoder->seekable();
    else
        return false;
}

bool BufferedDecoder::seek(long pos) {
    if (d->state == Closed) return false;
    if (!d->decoder->seekable()) return false;
    if (d->state == Open) {
        return d->decoder->seek(pos);
    }

    if (d->fading_time > 0 && !d->buffer->empty()) {
        d->xfader = new CrossFader(d->fading_time*2);
        fillFader();
        d->state = XFadingSeek;
    }

    d->seek_pos = pos;
    d->buffer->flush();

    // we have to assume the seek will go well at this point
    return true;
}

void BufferedDecoder::pause() {
    if (d->state == Closed || d->state == Open || d->state == Paused) return;
    /*
    if (d->state == Playing && !d->buffer->empty()) {
        d->xfader = new CrossFader(d->fading_time);
        fillFader();
        d->state = FadingOut;
    } */

    d->buffer->pause();

    d->state = Paused;
}

void BufferedDecoder::resume() {
    if (d->state != Paused);
    /*
    if (d->state == Playing || d->state == Paused || d->state == FadingOut) {
        d->xfader = new CrossFader(d->fading_time);
        d->state = FadingIn;
    } */

    d->buffer->resume();

    d->state = Playing;
}


void BufferedDecoder::setBufferSize(int size) {
    d->buffer_size = size;
    if (d->state == Open) {
        delete d->buffer;
        d->buffer = new AudioBuffer(d->buffer_size);
    }

}

void BufferedDecoder::setFadingTime(int time) {
    d->fading_time = time;
}

void BufferedDecoder::setBlockingRead(bool block) {
    d->blocking = block;
}

AudioBuffer * BufferedDecoder::buffer() const {
    return d->buffer;
}

const AudioConfiguration* BufferedDecoder::audioConfiguration() {
    // ### Might the buffer contain a different configuration?
    if (d->decoder)
        return d->decoder->audioConfiguration();
    else
        return 0;
}

void BufferedDecoder::fillFader() {
    if (!d->xfader) return;

    AudioFrame frame;
    while (true) // fill the crossfader with what might be in buffer
    {
        if (!d->buffer->get(&frame, false)) break;
        if (!d->xfader->writeFrame(&frame)) break;
    }
}

} // namespace
