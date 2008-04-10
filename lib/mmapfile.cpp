/*  aKode: MMapFile

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
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <string.h>

    #if defined(HAVE_MADVISE) && defined(NEED_MADVISE_PROTOTYPE)
    extern int madvise(void*,size_t,int);
    #endif
}

#include "mmapfile.h"

namespace aKode {

MMapFile::MMapFile(const char* filename) : File(filename), fd(-1), handle(0), len(0), pos(0) {}

MMapFile::~MMapFile() {
    close();
}

bool MMapFile::openRO() {
    if(handle) return true;
    struct stat stat;

    fd = ::open(filename, O_RDONLY);
    if (fstat(fd, &stat) < 0) {
        ::close(fd);
        return false;
    }
    len = stat.st_size;
    pos = 0;

    handle = mmap(0, len, PROT_READ, MAP_SHARED, fd, 0);
    if (handle == MAP_FAILED) {
        ::close(fd);
        handle = 0;
        return false;
    }

    return true;
}

void MMapFile::close() {
    if(handle) {
        munmap((char*)handle,len);
        ::close(fd);
    }
    handle = 0;
}

long MMapFile::read(char* ptr, long num) {
    if(!handle) return -1;

    if (pos+num > len) num = len-pos;
    memcpy(ptr, (char*)handle+pos, num);
    pos += num;

    return num;
}

long MMapFile::write(const char*, long) {
    return false;
}

bool MMapFile::seek(long to, int whence) {
    if(!handle) return false;

    long newpos = 0;
    switch (whence) {
        case SEEK_SET:
            newpos = to;
            break;
        case SEEK_CUR:
            newpos = pos + to;
            break;
        case SEEK_END:
            newpos = len + to;
            break;
        default:
            return false;
    }
    if (newpos > len || newpos < 0)
        return false;
    pos = newpos;
    return true;
}

long MMapFile::position() const {
    if(!handle) return -1;

    return pos;
}

long MMapFile::length() const {
    if(!handle) return -1;

    return len;
}

bool MMapFile::error() const {
    return (!handle);
}

bool MMapFile::eof() const {
    if(!handle) return true;
    return pos >= len;
}

void MMapFile::fadvise() {
    if(!handle) return;
    #if defined(HAVE_POSIX_MADVISE) && defined(POSIX_MADV_SEQUENTIAL)
    posix_madvise((char*)handle+pos, len-pos, POSIX_MADV_SEQUENTIAL);
    #elif defined(HAVE_MADVISE) && defined(MADV_SEQUENTIAL)
    madvise((char*)handle+pos, len-pos, MADV_SEQUENTIAL);
    #endif
}

} //namespace
