/*  aKode: Encoder abstract-type

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

#ifndef _AKODE_ENCODER_H
#define _AKODE_ENCODER_H

#include "pluginhandler.h"
#include "sink.h"

namespace aKode {

class File;

//! A generic interface for all encoders

/*!
 * Encoders are specialized sinks that can write to files. Be aware that encoders in aKode 1.0
 * cannot handle tags. So the files should be tagged with TagLib after encoding.
 */
class Encoder : public Sink {
public:
    /*!
     * Returns the current position in file/stream in milliseconds.
     * Notice that this position is not the necessarely the same as the
     * the one in the incoming stream.
     * Returns -1 if the position is unknown.
     */
    virtual long position() = 0;
    /*!
     * Returns true if the encoder has encountered a non-recoverable error
     */
    virtual bool error() = 0;
};

/*!
 * Parent class for encoder plugins
 */
class EncoderPlugin {
public:
    /*!
     * Asks the plugin to open a Encoder that writes to the file \a dst
     */
    virtual Encoder* openEncoder(File *dst) = 0;
};

/*!
 * Handler for encoder-plugins.
 */
class EncoderPluginHandler : public PluginHandler, public EncoderPlugin {
public:
    EncoderPluginHandler() : encoder_plugin(0) {};
    EncoderPluginHandler(const string name);
    Encoder* openEncoder(File *dst);
    /*!
     * Loads a encoder-plugin by the \a name (flac, vorbis, lame..)
     */
    virtual bool load(const string name);
    bool isLoaded() { return encoder_plugin != 0; };
protected:
    EncoderPlugin* encoder_plugin;
};

} // namespace

#endif
