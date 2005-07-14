/*  aKode: Decoder abstract-type

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

#ifndef _AKODE_DECODER_H
#define _AKODE_DECODER_H

#include "pluginhandler.h"
#include "akode_export.h"

namespace aKode {

class AudioConfiguration;

//! A generic interface for all decoders

/*!
 * There are two subtypes of decoders in aKodeLib: FrameDecoders and StreamDecoders.
 * This class defines the functions shared between them.
 */
class AKODE_EXPORT Decoder {
public:
    virtual ~Decoder() {};
    /*!
     * Returns the length of the file/stream in milliseconds.
     * Returns -1 if the length is unknown.
     */
    virtual long length() = 0;
    /*!
     * Returns the current position in file/stream in milliseconds.
     * Returns -1 if the position is unknown.
     */
    virtual long position() = 0;
    /*!
     * Attempts a seek to \a pos milliseconds into the file/stream.
     * Returns true if succesfull.
     */
    virtual bool seek(long pos) = 0;
    /*!
     * Returns true if the decoder is seekable
     */
    virtual bool seekable() = 0;
    /*!
     * Returns true if the decoder has reached the end-of-file/stream
     */
    virtual bool eof() = 0;
    /*!
     * Returns true if the decoder has encountered a non-recoverable error
     */
    virtual bool error() = 0;
    /*!
     * Returns the configuration of the decoded stream.
     * Returns 0 if unknown.
     */
    virtual const AudioConfiguration* audioConfiguration() = 0;
};

class FrameDecoder;
class StreamDecoder;
class File;

/*!
 * Parent class for decoder plugins
 */
class DecoderPlugin {
public:
    /*!
     * Asks the plugin to open a FrameDecoder, returns 0 if the
     * plugin only provides a StreamDecoder.
     */
    virtual FrameDecoder* openFrameDecoder(File *) { return 0; };
    /*!
     * Asks the plugin to open a StreamDecoder, returns 0 if the
     * plugin only provides a FrameDecoder.
     */
    virtual StreamDecoder* openStreamDecoder(File *) { return 0; };
};

/*!
 * Handler for decoder-plugins.
 */
class AKODE_EXPORT DecoderPluginHandler : public PluginHandler, public DecoderPlugin {
public:
    static list<string> listDecoderPlugins();

    DecoderPluginHandler() : decoder_plugin(0) {};
    DecoderPluginHandler(const string name);
    FrameDecoder* openFrameDecoder(File *src);
    StreamDecoder* openStreamDecoder(File *src);
    /*!
     * Loads a decoder-plugin named \a name (xiph, mpc, mpeg..)
     */
    virtual bool load(const string name);
    bool isLoaded() { return decoder_plugin != 0; };
protected:
    DecoderPlugin* decoder_plugin;
};

} // namespace

#endif
