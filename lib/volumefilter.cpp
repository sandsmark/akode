/*  aKode Volume-Filter

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

#include "audioframe.h"
#include "arithmetic.h"
#include "volumefilter.h"

#define VM_FIDELITY (1<<14)

namespace aKode {

VolumeFilter::VolumeFilter() : m_volume(0) {}

// T is the input/output type, S is the fast arithmetics type, Arithm is a division definition
template<typename T, typename S, template<typename S> class Arithm>
static bool _doFrame(AudioFrame* in, AudioFrame* out, int volume)
{
    T** indata = (T**)in->data;
    T** outdata = (T**)out->data;

    long length = in->length;
    if (in->channels != out->channels) return false;
    if (in->sample_width != out->sample_width) return false;

    S smax = Arithm<S>::max(in->sample_width);

    for(int i=0; i<in->channels; i++) {
        for(int j=0; j<length; j++) {
            S signal = Arithm<S>::muldiv(outdata[i][j], volume, VM_FIDELITY);

            if (signal > smax) signal = smax;
            else
            if (signal < -smax) signal = -smax;

            indata[i][j] = (T)(signal);
        }
    }
    return true;
}


bool VolumeFilter::doFrame(AudioFrame* in, AudioFrame* out)
{
    if (!out) out = in;

    int volint = (int)(m_volume*VM_FIDELITY+0.5);

    if (in->sample_width < -32) {
        return _doFrame<double, double, Arithm_FP>(in, out, volint);
    } else
    if (in->sample_width < 0) {
        return _doFrame<float,  float,  Arithm_FP>(in, out, volint);
    } else
    if (in->sample_width <= 8) {
        return _doFrame<int8_t, int32_t, Arithm_Int>(in, out, volint);
    } else
    if (in->sample_width <= 16) {
        return _doFrame<int16_t, int32_t, Arithm_Int>(in, out, volint);
    } else
    if (in->sample_width <= 24) {
        return _doFrame<int32_t, int32_t, Arithm_Int>(in, out, volint);
    } else
        return _doFrame<int32_t, int64_t, Arithm_Int>(in, out, volint);
}

void VolumeFilter::setVolume(float volume) {
    m_volume = volume;
}

float VolumeFilter::volume() const {
    return m_volume;
}

} // namespace
