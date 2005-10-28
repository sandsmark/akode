/*  aKode: Wav-Decoder

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

#ifndef _AKODE_WAV_DECODER_H
#define _AKODE_WAV_DECODER_H

#include "decoder.h"

namespace aKode {

class File;
class AudioFrame;

class WavDecoder : public Decoder {
public:
    WavDecoder(File* src);
    virtual ~WavDecoder();

    virtual bool openFile(File*);
    virtual void close();
    virtual bool readFrame(AudioFrame*);
    virtual long length();
    virtual long position();
    virtual bool eof();
    virtual bool error();
    virtual bool seek(long);
    virtual bool seekable();

    virtual const AudioConfiguration* audioConfiguration();

    struct private_data;
private:
    private_data *d;
};


class WavDecoderPlugin : public DecoderPlugin {
public:
    virtual bool canDecode(File*);
    virtual WavDecoder* openDecoder(File* str) {
        return new WavDecoder(str);
    };
};

extern "C" WavDecoderPlugin wav_decoder;

} // namespace

#endif
