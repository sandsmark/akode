/*  aKode: LocalFile

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "akodelib.h"

extern "C" {
    #ifdef HAVE_FEATURES_H
    // Needed for older glibc versions.
    #include <features.h>
    #ifndef __USE_XOPEN2K
    #define __USE_XOPEN2K
    #endif
    #endif

    #include <unistd.h>
    #ifdef HAVE_SYS_TYPES_H
    #include <sys/types.h>
    #endif
    #include <sys/stat.h>
    #include <fcntl.h>
}

#include "localfile.h"

namespace aKode {

LocalFile::LocalFile(const char* filename) : File(filename)
                    , _fd(-1), pos(0), len(0)
                    , m_readable(false), m_writeable(false)
                    , m_eof(false)
                    {}

LocalFile::~LocalFile() {
    close();
}

bool LocalFile::openRO() {
    if(_fd != -1) {
        return seek(0) && m_readable;
    }

    struct stat stat;
    _fd = ::open(filename, O_RDONLY);
    if (fstat(_fd, &stat) < 0) {
        ::close(_fd);
        return false;
    }
    len = stat.st_size;

    m_readable = true;
    m_writeable = false;
    m_eof = false;
    return ( _fd != -1 );
}

bool LocalFile::openRW() {
    if(_fd != -1) {
        return seek(0) && m_readable && m_writeable;
    }

    struct stat stat;
    _fd = ::open(filename, O_RDWR);
    if (fstat(_fd, &stat) < 0) {
        ::close(_fd);
        return false;
    }
    len = stat.st_size;

    m_readable = true;
    m_writeable = true;
    return ( _fd != -1 );
}

bool LocalFile::openWO() {
    if(_fd != -1) {
        return seek(0) && m_writeable;
    }

    _fd = ::open(filename, O_WRONLY);

    m_readable = false;
    m_writeable = true;
    return ( _fd != -1 );
}

void LocalFile::close() {
    if(_fd != -1) ::close(_fd);
    _fd = -1;
}

long LocalFile::read(char* ptr, long num) {
    if(_fd == -1 || !m_readable) return -1;
    long n = ::read(_fd, ptr, num);
    pos += n;
    m_eof = (n == 0 && num != 0);
    return n;
}

long LocalFile::write(const char* ptr, long num) {
    if(_fd == -1 || !m_writeable) return -1;
    long n = ::write(_fd, ptr, num);
    pos += n;
    if (pos > len) len = pos;
    return n;
}

bool LocalFile::seek(long to, int whence) {
    if(_fd == -1) return false;
    int s = ::lseek(_fd, to, whence);
    if (s >= 0) pos = s;
    return (s >= 0);
}

long LocalFile::position() const {
    if(_fd == -1) return -1;
    return pos;
}

long LocalFile::length() const {
    if(_fd == -1) return -1;
    return len;
}

bool LocalFile::eof() const {
    if(_fd == -1) return true;
    return m_eof || (pos >= len);
}

bool LocalFile::error() const {
    return (_fd == -1);
}

int LocalFile::fd() const {
    return _fd;
}

void LocalFile::fadvise() {
    if(_fd == -1) return;
    #if defined(HAVE_POSIX_FADVISE) && defined(POSIX_FADV_SEQUENTIAL)
    posix_fadvise(_fd, pos, len-pos, POSIX_FADV_SEQUENTIAL);
    #elif defined(HAVE_FADVISE) && defined(FADV_SEQUENTIAL)
    fadvise(_fd, pos, len-pos, FADV_SEQUENTIAL);
    #endif
}

} // namespace
