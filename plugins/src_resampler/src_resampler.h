/*  aKode: Resampler (Secret Rabbit Code)

    Copyright (C) 2004 Allan Sandfeld Jensen <kde@carewolf.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston,
    MA  02110-1301, USA.
*/
#ifndef _AKODE_SRC_RESAMPLER_H
#define _AKODE_SRC_RESAMPLER_H

#include "akodelib.h"

#include "resampler.h"

namespace aKode {

class AudioFrame;

//! High quality resampler using the Secret Rabbit Code library also known as libsamplerate

/*!
 * This is the high quality resampler. Compared to the default resampler, it is
 * lot slower, uses primarely float point, and is GPL.
 */
class SRCResampler : public Resampler {
public:
    SRCResampler();
    bool doFrame(AudioFrame* in, AudioFrame* out);
    void setSampleRate(unsigned int rate);
    void setSpeed(float speed);

    float speed;
    unsigned int sample_rate;
};

class SRCResamplerPlugin : public ResamplerPlugin {
public:
    virtual SRCResampler* openResampler();
};

extern "C" SRCResamplerPlugin src_resampler;

} // namespace

#endif
