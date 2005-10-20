/*  aKode: LocalFile-type

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

#ifndef _AKODE_LOCALFILE_H
#define _AKODE_LOCALFILE_H

#include "file.h"
#include "akode_export.h"
namespace aKode {

//! An implementation of the File interface for local-file access

/*!
 * LocalFile uses the ordinary system-interfaces for file-access, and is
 * thus capable of both reading, writing and seeking.
 */
class AKODE_EXPORT LocalFile : public File {
    int _fd;
    long pos;
    long len;
    bool m_readable;
    bool m_writeable;
    bool m_eof;
public:
    LocalFile(const char* filename);
    virtual ~LocalFile();

    bool openRO();
    bool openRW();
    bool openWO();
    void close();

    long read(char* ptr, long num);
    long write(const char* ptr, long num);

    bool seek(long to, int whence = SEEK_SET);
    long position() const;
    long length() const;

    bool seekable() const { return true; };
    bool readable() const { return m_readable; };
    bool writeable() const { return m_writeable; };

    bool error() const;
    bool eof() const;

    void fadvise();

    int fd() const;
};

} //namespace

#endif
