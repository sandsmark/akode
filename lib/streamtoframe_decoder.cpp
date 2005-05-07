/*  aKode: Stream-to-Frame Decoder

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

#include "audioframe.h"
#include "audiobuffer.h"
#include "streamdecoder.h"
#include "framedecoder.h"
#include "crossfader.h"
#include "streamtoframe_decoder.h"

namespace aKode {

struct StreamToFrameDecoder::private_data
{
    private_data() : buffer(0)
                   , inDecoder(0)
                   , xfader(0)
                   , latestPos(-1)
                   , halted(false)
                   , halting(false)
                   , blocking(false) {};
    AudioBuffer *buffer;
    StreamDecoder *inDecoder;
    CrossFader *xfader;
    long latestPos;
    AudioConfiguration latest_config;
    bool halted, halting;
    bool blocking;
};

StreamToFrameDecoder::StreamToFrameDecoder(StreamDecoder *inDecoder, AudioBuffer *buffer)
{
    m_data = new private_data;
    m_data->inDecoder = inDecoder;
    m_data->buffer = buffer;
    inDecoder->readStream(buffer);
}

StreamToFrameDecoder::~StreamToFrameDecoder() {
    m_data->inDecoder->halt();
    delete m_data;
}

bool StreamToFrameDecoder::readFrame(AudioFrame* frame)
{
    if (m_data->halted) return false;

    if (m_data->halting) {
        if (!m_data->xfader->full()) {
            fillFader();
        }
        if (!m_data->xfader->readFrame(frame)) {
            m_data->halting = false;
            m_data->halted = true;
            m_data->buffer->flush();
            m_data->inDecoder->halt();
            return false;
        }
        return true;
    }
    // Get non-blocking
    if (m_data->buffer->get(frame, m_data->blocking)) {
        m_data->latestPos = frame->pos;
        m_data->latest_config = *frame;

        if (m_data->xfader) {
            if(!m_data->xfader->doFrame(frame)) {
                delete m_data->xfader;
                m_data->xfader = 0;
            }
        }
        return true;
    }
    else
        return false;
}

long StreamToFrameDecoder::length() {
    return m_data->inDecoder->length();
}

long StreamToFrameDecoder::position() {
    if (m_data->latestPos>0)
        return m_data->latestPos;
    else
        return m_data->inDecoder->position();
}

bool StreamToFrameDecoder::eof() {
    return m_data->halted || (m_data->inDecoder->eof() && m_data->buffer->empty());
}

bool StreamToFrameDecoder::error() {
    return m_data->inDecoder->error();
}

bool StreamToFrameDecoder::seek(long pos) {
    if(m_data->inDecoder->seek(pos)) {
        AudioFrame frame;
        m_data->xfader = new CrossFader(100);
        fillFader();

        m_data->buffer->flush();
        m_data->latestPos = -1;
        return true;
    } else
        return false;
}

void StreamToFrameDecoder::stop() {
    if (m_data->inDecoder->eof() && m_data->buffer->empty())
        return;
    m_data->xfader = new CrossFader(50);
    fillFader();
    m_data->halting = true;
}

void StreamToFrameDecoder::resume() {
    m_data->halted = false;
    m_data->halting = false;
}

void StreamToFrameDecoder::fillFader() {
    if (!m_data->xfader) return;

    AudioFrame frame;
    while (true) // fill the crossfader with what might be in buffer
    {
        if (!m_data->buffer->get(&frame, false)) break;
        if (!m_data->xfader->writeFrame(&frame)) break;
    }
}

void StreamToFrameDecoder::setBlocking(bool block) {
    m_data->blocking = block;
}

const AudioConfiguration* StreamToFrameDecoder::audioConfiguration() {
    if (m_data->latest_config.channels > 0)
        return &m_data->latest_config;
    else
        return m_data->inDecoder->audioConfiguration();
}

} // namespace
