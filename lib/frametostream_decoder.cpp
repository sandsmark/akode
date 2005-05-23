/*  aKode: Frame-to-Stream Decoder

    Copyright (C) 2004 Allan Sandfeld Jensen <kde@carewolf.com>

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

#include "audioframe.h"
#include "audiobuffer.h"
#include "streamdecoder.h"
#include "framedecoder.h"
#include "frametostream_decoder.h"

// #include <iostream>

namespace aKode {

struct FrameToStreamDecoder::private_data
{
    private_data() : buffer(0), inDecoder(0), running(false), error(false), halt(false), seek_pos(-1) {};
    AudioBuffer *buffer;
    FrameDecoder *inDecoder;
    bool running, error;
    volatile bool halt;
    volatile long seek_pos;
    pthread_t streamDecoder;
};

// The decoder-thread. It is controlled through the two variables
// halt and seek_pos in m_data
static void* run_decoder(void* arg) {
    FrameToStreamDecoder::private_data *m_data = (FrameToStreamDecoder::private_data*)arg;

    AudioFrame frame;
    bool no_error;
    m_data->halt = false;
    m_data->seek_pos = -1;

    while(true) {
        no_error = m_data->inDecoder->readFrame(&frame);
        if (no_error)
            m_data->buffer->put(&frame, true);
        else {
            if (m_data->inDecoder->error() || m_data->inDecoder->eof()) {
                break;
            }
//             std::cerr << "akode: [FtS] Blop?\n";
        }

        if (m_data->halt) break;
        if (m_data->seek_pos>=0) {
            m_data->inDecoder->seek(m_data->seek_pos);
            m_data->seek_pos = -1;
        }
    }

//     std::cerr << "akode: [FtS] EOF\n";
    m_data->buffer->setEOF();
    m_data->halt = true;
    m_data->buffer = 0;
    return (void*)0;
}

FrameToStreamDecoder::FrameToStreamDecoder(FrameDecoder *inDecoder){
    m_data = new private_data;
    m_data->inDecoder = inDecoder;
}

FrameToStreamDecoder::~FrameToStreamDecoder() {
    halt();
    delete m_data;
}

void FrameToStreamDecoder::readStream(AudioBuffer* buffer)
{
    if (!buffer) { m_data->error = true; return; };

    m_data->buffer = buffer;
    if (pthread_create(&m_data->streamDecoder, 0, run_decoder, m_data) == 0) {
        m_data->running = true;
    }
}

long FrameToStreamDecoder::length() {
    return m_data->inDecoder->length();
}

long FrameToStreamDecoder::position() {
    if (m_data->seek_pos > 0 )
        return m_data->seek_pos;
    else
        return m_data->inDecoder->position();
}

bool FrameToStreamDecoder::eof() {
    return m_data->halt || m_data->inDecoder->eof();
}

bool FrameToStreamDecoder::error() {
    return m_data->error || m_data->inDecoder->error();
}

bool FrameToStreamDecoder::seek(long pos) {
    m_data->seek_pos = pos;
    return true;
}

void FrameToStreamDecoder::halt() {
    m_data->halt = true;
    if (m_data->buffer) {
        m_data->buffer->release();
    }
    if (m_data->running) {
        pthread_join(m_data->streamDecoder, 0);
        m_data->running = false;
    }
    m_data->buffer = 0;
}

const AudioConfiguration* FrameToStreamDecoder::audioConfiguration() {
    return m_data->inDecoder->audioConfiguration();
}

} // namespace
