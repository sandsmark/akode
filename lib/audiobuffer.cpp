/*  aKode AudioBuffer

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

#include "audiobuffer.h"
#include "audioframe.h"

namespace aKode {

AudioBuffer::AudioBuffer(unsigned int len) : length(len), readPos(0), writePos(0),
    flushed(false), released(false), paused(false), m_eof(false)
{
    pthread_cond_init(&not_empty, 0);
    pthread_cond_init(&not_full, 0);
    pthread_mutex_init(&mutex, 0);
    buffer = new AudioFrame[len];
}

AudioBuffer::~AudioBuffer() {
    delete[] buffer;
}

bool AudioBuffer::put(AudioFrame* buf, bool blocking) {
    pthread_mutex_lock(&mutex);
    if (released) goto fail;
    flushed = false;
    if ((writePos+1) % length == readPos) {
        if (blocking) {
            pthread_cond_wait(&not_full, &mutex);
            if (flushed || released) goto fail;
        }
        else
            goto fail;
    }

    takeover(&buffer[writePos], buf);
    writePos = (writePos+1) % length;

    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&mutex);
    return true;
fail:
    pthread_mutex_unlock(&mutex);
    return false;
}

bool AudioBuffer::get(AudioFrame* buf, bool blocking) {
    pthread_mutex_lock(&mutex);
    if (released) goto fail;
    if (readPos == writePos || paused) {
        if (blocking && !m_eof) {
            pthread_cond_wait(&not_empty, &mutex);
            if (released) goto fail;
            if (empty()) goto fail;
        }
        else
            goto fail;
    }

    takeover(buf, &buffer[readPos]);
    readPos = (readPos+1) % length;

    pthread_cond_signal(&not_full);
    pthread_mutex_unlock(&mutex);
    return true;
fail:
    pthread_mutex_unlock(&mutex);
    return false;
}

long AudioBuffer::position() {
    long out = -1;
    pthread_mutex_lock(&mutex);
    if (!empty() && !released)
        out = buffer[readPos].pos;
    pthread_mutex_unlock(&mutex);
    return out;
}

bool AudioBuffer::empty() {
    return (readPos == writePos);
}

bool AudioBuffer::full() {
    return (readPos == (writePos+1) % length);
}

void AudioBuffer::setEOF() {
    pthread_mutex_lock(&mutex);
    m_eof = true;
    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&mutex);
}

bool AudioBuffer::eof() {
    return m_eof && empty();
}

void AudioBuffer::reset() {
    // We assume all processes have been released at this point
    readPos = writePos = 0;
    flushed = released = paused = m_eof = false;
}

void AudioBuffer::flush() {
    pthread_mutex_lock(&mutex);
    // Don't free the frames, most likely this is just a seek
    // and the same buffer-sizes will be needed afterwards.
    readPos = writePos = 0;
    flushed = true;
    pthread_cond_signal(&not_full);
    pthread_mutex_unlock(&mutex);
}

void AudioBuffer::release() {
    pthread_mutex_lock(&mutex);
    released = true;
    pthread_cond_signal(&not_full);
    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&mutex);
}

void AudioBuffer::pause() {
    paused = true;
}

void AudioBuffer::resume() {
    pthread_mutex_lock(&mutex);
    paused = false;
    if (!empty())
        pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&mutex);
}

} // namespace
