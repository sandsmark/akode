/*  aKode: Buffered Decoder

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

#ifndef _AKODE_BUFFERED_DECODER_H
#define _AKODE_BUFFERED_DECODER_H

#include "decoder.h"
#include "akode_export.h"

namespace aKode {

class AudioBuffer;
class AudioConfiguration;

class AKODE_EXPORT BufferedDecoder : public Decoder {
public:
    BufferedDecoder();
    virtual ~BufferedDecoder();

    void openDecoder(Decoder*);
    void closeDecoder();

    void start();
    void stop();

    void pause();
    void resume();

    virtual bool readFrame(AudioFrame*);
    virtual bool seek(long pos);

    virtual long length();
    virtual long position();
    virtual bool seekable();

    virtual bool eof();
    virtual bool error();

    void setBlockingRead(bool block);

    void setBufferSize(int size);
    void setFadingTime(int time);

//     void setFadeInSeek(bool fade);
//     void setFadeToStop(bool fade);

    AudioBuffer* buffer() const;

    virtual const AudioConfiguration* audioConfiguration();

    struct private_data;
private:
    private_data *d;

protected:
    void fillFader();
};

} // namespace

#endif
