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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef _AKODE_AUDIOBUFFER_H
#define _AKODE_AUDIOBUFFER_H

#include <pthread.h>

#include "audioframe.h"
#include "akode_export.h"

namespace aKode {

//! A reentrant circular buffer of AudioFrames

/*!
 * A buffer of AudioFrame to synchronize audio between two threads, one putting and one getting.
 */
class AKODE_EXPORT AudioBuffer {
    const unsigned int length;
    AudioFrame* buffer;
    volatile unsigned int readPos;
    volatile unsigned int writePos;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
    pthread_mutex_t mutex;
    volatile bool flushed, released, paused, m_eof;
    void takeover(AudioFrame* toFrame, AudioFrame* fromFrame) {
        AudioFrame tmpFrame;

        tmpFrame = *toFrame;
        *toFrame = *fromFrame;
        *fromFrame = tmpFrame;
        tmpFrame.data = 0;
    }
public:
    /*!
     * Constructs a buffer with \a len AudioFrames.
     */
    AudioBuffer(unsigned int len);
    ~AudioBuffer();

    /*!
     * Puts the AudioFrame \a buf into the buffer. If blocking is set to true,
     * put will block until the buffer is not full.
     * Returns true if succesfull.
     *
     * \note That put will fail even if blocking is set, if flush or release is called.
     */
    bool put(AudioFrame* buf, bool blocking = false);
    /*!
     * Gets one AudioFrame from the buffer. If blocking is set to true,
     * get will block until the buffer is not empty.
     * Returns true if succesfull.
     *
     * \note That get will fail even if blocking is set if release is called.
     */
    bool get(AudioFrame* buf, bool blocking = false);

    /*!
     * Returns the position of the next audioFrame in the buffer
     */
    long position();

    /*!
     * Returns true if the buffer is empty.
     */
    bool empty();
    /*!
     * Returns true if the buffer is full.
     */
    bool full();

    /*!
     * Sets EOF for the incoming stream. Releases any blocking get-calls.
     */
    void setEOF();
    /*!
     * Returns true if the sender has set the EOF flag.
     */
    bool eof();

    /*!
     * Resets the buffer to be as good as new.
     */
    void reset();
    /*!
     * Flushes the buffer and releases any blocking put-calls.
     */
    void flush();
    /*!
     * Releases all blocking threads and prepares the buffer for deletion.
     * Use reset to make the buffer usable again.
     */
    void release();
    /*!
     * Blocks the buffer for reading.
     */
    void pause();
    /*!
     * Un-blocks the buffer for reading.
     */
    void resume();
};

} // namespace

#endif
