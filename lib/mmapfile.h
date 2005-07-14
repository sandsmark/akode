/*  aKode: MMapFile-type

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

#ifndef _AKODE_MMAPFILE_H
#define _AKODE_MMAPFILE_H

#include "file.h"
#include "akode_export.h"
namespace aKode {

//! An implementation of the File interface that uses mmap

/*!
 * MMapFile uses the mmap system-call for file-access.
 * This is often faster than ordinary file-access. In return
 * MMapFile does not provide writing capabilities.
 */
class AKODE_EXPORT MMapFile : public File {
    int fd;
    void* handle;
    long len;
    long pos;
public:
    MMapFile(const char* filename);
    virtual ~MMapFile();

    bool openRO();
    void close();

    long read(char* ptr, long num);
    long write(const char*, long);

    bool seek(long to, int whence = SEEK_SET);
    long position() const;
    long length() const;

    bool seekable() const { return true; };
    bool readable() const { return true; };
    bool writeable() const { return false; };

    bool eof() const;
    bool error() const;

    void fadvise();
};

} //namespace

#endif
