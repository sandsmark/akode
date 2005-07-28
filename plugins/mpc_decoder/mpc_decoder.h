/*  aKode: MPC-Decoder

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

#ifndef _AKODE_MPC_DECODER_H
#define _AKODE_MPC_DECODER_H

#include <config.h>
//#ifdef AKODE_COMPILE_MPC

#include "decoder.h"

namespace aKode {

class File;
class AudioBuffer;

class MPCDecoder : public Decoder {
public:
    MPCDecoder(File* src);
    virtual ~MPCDecoder();

    virtual void initialize();
    virtual bool readFrame(AudioFrame* frame);
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
};


class MPCDecoderPlugin : public DecoderPlugin {
public:
    virtual bool canDecode(File*);
    virtual MPCDecoder* openDecoder(File* src) {
        return new MPCDecoder(src);
    };
};

extern "C" MPCDecoderPlugin mpc_decoder;

} // namespace

//#endif // AKODE_COMPILE_MPC
#endif
