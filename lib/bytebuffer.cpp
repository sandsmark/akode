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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#include <string.h>

#include "bytebuffer.h"

namespace aKode {

ByteBuffer::ByteBuffer(unsigned int len)
              : length(len)
              , readPos(0)
              , writePos(0)
              , flushed(false)
              , released(false)
              , closed(false)
{
    pthread_cond_init(&not_empty, 0);
    pthread_cond_init(&not_full, 0);
    pthread_mutex_init(&mutex, 0);
    buffer = new char[length];
}

ByteBuffer::~ByteBuffer() {
    delete[] buffer;
}

int ByteBuffer::write(char* buf, unsigned int len, bool blocking)
{
    pthread_mutex_lock(&mutex);
    if (released) len = 0;

    flushed = closed = false;
    while (space() < len) {
        if (blocking) {
            pthread_cond_wait(&not_full, &mutex);
            if (flushed || released) len = 0;
        }
        else
            len = space();
    }

    unsigned int base = len, rem = 0;
    if (writePos+len > length) {
        base = length-writePos;
        rem = len-base;
    }

    memcpy(buffer+writePos, buf, base);
    memcpy(buffer, buf+base, rem);

    writePos = (writePos+len) % length;

    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&mutex);
    return len;
}

int ByteBuffer::read(char* buf, unsigned int len, bool blocking)
{
    pthread_mutex_lock(&mutex);
    if (released) len = 0;
    if (closed) blocking = false;

    while (content() < len) {
        if (blocking) {
            pthread_cond_wait(&not_empty, &mutex);
            if (released)
                len = 0;
            else if (closed)
                len = content();
        }
        else
            len = content();
    }

    unsigned int base = len, rem = 0;
    if (readPos+len > length) {
        base = length-readPos;
        rem = len-base;
    }

    memcpy(buf, buffer+readPos, base);
    memcpy(buf+base, buffer, rem);

    readPos = (readPos+len) % length;

    pthread_cond_signal(&not_full);
    pthread_mutex_unlock(&mutex);
    return len;
}

void ByteBuffer::close() {
    pthread_mutex_lock(&mutex);
    closed = true;
    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&mutex);
}


bool ByteBuffer::eof() {
    return empty() && closed;
}

bool ByteBuffer::empty() {
    return (readPos == writePos);
}

bool ByteBuffer::full() {
    return (readPos == (writePos+1) % length);
}

unsigned int ByteBuffer::content() {
    unsigned int cn;
    if (readPos <= writePos)
        cn=writePos-readPos;
    else
        cn=writePos+length-readPos;
    return cn;
}

unsigned int ByteBuffer::space() {
    unsigned int sp = length - content() - 1;
    return sp;
}

void ByteBuffer::flush() {
    pthread_mutex_lock(&mutex);
    readPos = writePos = 0;
    flushed = true;
    pthread_cond_signal(&not_full);
    pthread_mutex_unlock(&mutex);
}

void ByteBuffer::release() {
    pthread_mutex_lock(&mutex);
    released = true;
    pthread_cond_signal(&not_empty);
    pthread_cond_signal(&not_full);
    pthread_mutex_unlock(&mutex);
}

} // namespace

