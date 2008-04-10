/*  aKode: File abstract-type

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

#ifndef _AKODE_FILE_H
#define _AKODE_FILE_H

extern "C" {
#include <sys/types.h>
#include <unistd.h>
}

namespace aKode {

//! The generic interface to aKode's virtualised files

/*!
 * An interface with the all the needed file/stream-operations
 * the library needs.
 */
class File {
public:
   /*!
    * Creates a File with the given \a filename.
    */
    File(const char* filename) : filename(filename) {};
    virtual ~File() {};
   /*!
    * Opens the file as read-only.
    * Returns whether the operation was succesfull.
    */
    virtual bool openRO() { return false; };
   /*!
    * Opens the file as read and write.
    * Returns whether the operation was succesfull.
    */
    virtual bool openRW() { return false; };
   /*!
    * Open the file as write-only.
    * Returns whether the operation was succesfull.
    */
    virtual bool openWO() { return false; };
   /*!
    * Closes the file.
    */
    virtual void close() = 0;
   /*!
    * Reads \a num bytes from the file and places the data in \a ptr.
    * Returns number of read characters. Returns 0 if end-of-file.
    * Returns negative at errors.
    */
    virtual long read(char* ptr, long num) = 0;
   /*!
    * Writes \a num bytes from \a ptr to the file.
    * Returns number of writen characters.
    * Returns negative at errors.
    */
    virtual long write(const char* ptr, long num) = 0;
   /*!
    * Seeks to the position.
    * Behaves semantically as lseek.
    */
    virtual bool seek(long to, int whence = SEEK_SET) = 0;
   /*!
    * Returns current position in file, or negative if unknown.
    */
    virtual long position() const = 0;
   /*!
    * Returns length of file, or negative if unknown.
    */
    virtual long length() const = 0;
   /*!
    * Returns true if the file is seekable
    */
    virtual bool seekable() const = 0;
   /*!
    * Returns true if the file is readable
    */
    virtual bool readable() const = 0;
   /*!
    * Returns true if the file is writeable
    */
    virtual bool writeable() const = 0;
   /*!
    * Returns whether the file has reached end-of-file.
    */
    virtual bool eof() const = 0;
   /*!
    * Returns true if the file has entered a non-recoverable error-state
    */
    virtual bool error() const = 0;
   /*!
    * Advises the OS (if possible), that the file will be read
    * sequentially from the current position and forward.
    */
    virtual void fadvise() = 0;
   /*!
    * The name of the file.
    */
    const char *filename;
};

} //namespace

#endif
