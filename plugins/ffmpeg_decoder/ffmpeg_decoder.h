/*  aKode: FFMPEG Decoder

    Copyright (C) 2005 Allan Sandfeld Jensen <kde@carewolf.com>

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

#ifndef _AKODE_FFMPEG_DECODER_H
#define _AKODE_FFMPEG_DECODER_H

#include "akodelib.h"
// #ifdef HAVE_FFMPEG

#include "decoder.h"

namespace aKode {

class File;
class AudioFrame;

class FFMPEGDecoder : public Decoder {
public:
    FFMPEGDecoder(File* src);
    virtual ~FFMPEGDecoder();

    virtual bool openFile();
    void closeFile();

    virtual bool readFrame(AudioFrame*);
    virtual long length();
    virtual long position();
    virtual bool seek(long);
    virtual bool seekable();
    virtual bool eof();
    virtual bool error();

    virtual const AudioConfiguration* audioConfiguration();

    struct private_data;
private:
    bool readPacket();
    private_data *d;
};


class FFMPEGDecoderPlugin : public DecoderPlugin {
public:
    virtual bool canDecode(File*);
    virtual FFMPEGDecoder* openDecoder(File* src) {
        return new FFMPEGDecoder(src);
    };
};

extern "C" FFMPEGDecoderPlugin ffmpeg_decoder;

} // namespace

// #endif // HAVE_FFMPEG
#endif
