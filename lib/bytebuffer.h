/*  aKode: ByteBuffer

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

#ifndef _AKODE_BYTEBUFFER_H
#define _AKODE_BYTEBUFFER_H

#include <pthread.h>

#include "akode_export.h"

namespace aKode {

//! A reentrant circular buffer of bytes

/*!
 * A buffer of bytes to synchronize I/O between two threads, one reading and one getting.
 */
class AKODE_EXPORT ByteBuffer {
    const unsigned int length;
    char* buffer;
    volatile unsigned int readPos;
    volatile unsigned int writePos;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
    pthread_mutex_t mutex;
    volatile bool flushed, released, closed;
public:
    /*!
     * Constructs a buffer with \a len bytes.
     */
    ByteBuffer(unsigned int len);
    ~ByteBuffer();

    /*!
     * Write \a len bytes from \a buf into the buffer. If blocking is set to true,
     * write will block until all bytes have been writen or release() or flush()
     * is called.
     * Returns the number of bytes writen.
     */
    int write(char* buf, unsigned int len, bool blocking = false);
    /*!
     * Read \a len bytes from the buffer into \a buf. If blocking is set to true,
     * read will block until \a len bytes have been read or release()
     * is called.
     * Returns the number of bytes read.
     */
    int read(char* buf, unsigned int len, bool blocking = false);

    /*!
     * Called by the writing thread to denote EOF
     */
    void close();

    /*!
     * Returns true if the stream is empty and closed
     */
    bool eof();

    /*!
     * Returns true if the buffer is empty
     */
    bool empty();
    /*!
     * Returns true if the buffer is full
     */
    bool full();

    /*!
     * Returns the number of bytes that can be read without blocking
     */
    unsigned int content();
    /*!
     * Returns the number of bytes that can be writen without blocking
     */
    unsigned int space();

    /*!
     * Flushes the buffer and releases any blocking write-calls.
     */
    void flush() ;
    /*!
     * Releases all blocking threads and prepares the buffer for deletion.
     */
    void release();
};

} // namespace

#endif
