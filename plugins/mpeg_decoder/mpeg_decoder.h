/*  aKode: MPEG-Format(MAD)

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

#ifndef _AKODE_MPEG_DECODER_H
#define _AKODE_MPEG_DECODER_H

#include "decoder.h"

namespace aKode {

class File;
class AudioFrame;

class MPEGDecoder : public Decoder {
public:
    MPEGDecoder(File* src);
    virtual ~MPEGDecoder();

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
    private_data *m_data;
    bool prepare();
    bool sync();
    bool skipID3v2();
    bool moreData(bool flush = false);
};


class MPEGDecoderPlugin : public DecoderPlugin {
public:
    virtual bool canDecode(File*);
    virtual MPEGDecoder* openDecoder(File* src)
    {
        return new MPEGDecoder(src);
    };
};

extern "C" MPEGDecoderPlugin mpeg_decoder;

} // namespace

#endif
