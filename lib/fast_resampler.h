/*  aKode: Resampler (fast)

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
#ifndef _AKODE_FAST_RESAMPLER_H
#define _AKODE_FAST_RESAMPLER_H

#include "resampler.h"

namespace aKode {

class AudioFrame;

//! Fast low quality resampler using linear interpolation

/*!
 * This is the default resampler, which excels over the SRCResampler
 * in being LGPL, fast and integer only, but the result is more
 * noisy.
 *
 * The quality of resampling most relevant to low signal samples.
 * For samples of 44100Hz/16bit or more, it is mostly irrelevant.
 */

class FastResampler : public Resampler {
public:
    FastResampler();
    bool doFrame(AudioFrame* in, AudioFrame* out);
    void setSampleRate(unsigned int rate);
    void setSpeed(float speed);

    float speed;
    unsigned int sample_rate;
};

class FastResamplerPlugin : public ResamplerPlugin {
public:
    virtual FastResampler* openResampler();
};

extern "C" FastResamplerPlugin fast_resampler;

} // namespace

#endif
