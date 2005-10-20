/*  aKode Resampler (fast)

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
#include "fast_resampler.h"

#define FR_FIDELITY (1<<10)

namespace aKode {

FastResampler* FastResamplerPlugin::openResampler() {
    return new FastResampler();
}

extern "C" { FastResamplerPlugin fast_resampler; }

FastResampler::FastResampler() : speed(1.0), sample_rate(44100) {}

// A fast resampling by linear interpolation
// I assume you know binary arithmetics and convertions if you're reading this
// T is the input/output type, Arithm defines the used arithmetic
template<typename T, typename S, template<typename S> class Arithm>
static bool _doBuffer(AudioFrame* in, AudioFrame* out, float speed, unsigned sample_rate)
{
    unsigned long vt_pos_start = 0;  // virtual positions of new sample
    unsigned long vt_pos_end;;
    unsigned long real_pos_start, real_pos_end;
    long start_fraction, end_fraction;

    unsigned long vt_end = in->length * 1024 -1;

    int int_speed;
    unsigned long out_length;
    {
        float resample_speed = in->sample_rate/(float)sample_rate;
        resample_speed *= speed;
        int_speed = (unsigned int)(resample_speed*1024.0+0.5);

        out_length = (in->length/int_speed)*1024;
        unsigned long out_rem = (in->length%int_speed)*1024 + (int_speed-1);
        out_length += out_rem/int_speed;

        out->reserveSpace(in, out_length);
        out->sample_rate = sample_rate;
    }
    vt_pos_end = int_speed;

    unsigned char channels = in->channels; // take copy to reduce alias-induced reads
    unsigned long out_pos = 0;

    T** indata = (T**)in->data;    // templating ;)
    T** outdata = (T**)out->data;

    S sspeed = (S)int_speed;
    S smax = Arithm<S>::max(in->sample_width);

    while(out_pos < out_length && vt_pos_start < vt_end) {
        real_pos_start = vt_pos_start / 1024;     start_fraction = vt_pos_start % 1024;
        real_pos_end   = vt_pos_end   / 1024;       end_fraction = vt_pos_end   % 1024;

        if (real_pos_start == real_pos_end) {
            for(int i=0; i<channels; i++) {
                outdata[i][out_pos] = indata[i][real_pos_start];
            }
        } else {
            for(int i=0; i<channels; i++) {
                S signal = 0;
                S remainder = 0;
                S temp;

                // add fraction from starting sample
                temp = (S)indata[i][real_pos_start];
                signal    += Arithm<S>::div(temp,sspeed) * (1024L-start_fraction);
                remainder += Arithm<S>::rem(temp,sspeed) * (1024L-start_fraction);

                // add fraction from end sample
                temp= (S)indata[i][real_pos_end];
                signal    += Arithm<S>::div(temp,sspeed) * (end_fraction);
                remainder += Arithm<S>::rem(temp,sspeed) * (end_fraction);

                // add all intermediate samples
                for(unsigned long j = real_pos_start+1; j<real_pos_end; j++) {
                    temp= (S)indata[i][j];
                    signal += Arithm<S>::div(temp,sspeed) * 1024L;
                    remainder += Arithm<S>::rem(temp,sspeed) * 1024L;
                }

                // add accummulated remainder
                signal += Arithm<S>::div(remainder,sspeed);

                // clip to maximum, just to be safe
                if (signal > smax) signal = smax;
                else
                if (signal < -smax) signal = -smax;

                outdata[i][out_pos] = (T)signal;
            }
        }
        out_pos++;
        vt_pos_start = vt_pos_end;
        vt_pos_end += int_speed;
        if (vt_pos_end > vt_end) vt_pos_end = vt_end;
    }
    return true;
}

bool FastResampler::doFrame(AudioFrame* in, AudioFrame* out)
{
    // FIXME: Do fast short-cut
    //if (speed == 1.0 && in->sample_rate == sample_rate)
    if (in->sample_width < 0) {
        return _doBuffer<float, float, Arithm_FP>(in, out, speed, sample_rate);
    } else
    if (in->sample_width <= 8) {
        return _doBuffer<int8_t, int32_t, Arithm_Int>(in, out, speed, sample_rate);
    } else
    if (in->sample_width <= 16) {
        return _doBuffer<int16_t, int32_t, Arithm_Int>(in, out, speed, sample_rate);
    } else
    if (in->sample_width <= 24) {
        return _doBuffer<int32_t, int32_t, Arithm_Int>(in, out, speed, sample_rate);
    } else
        return _doBuffer<int32_t, int64_t, Arithm_Int>(in, out, speed, sample_rate);
}

void FastResampler::setSpeed(float _speed)
{
    speed = _speed;
}

void FastResampler::setSampleRate(unsigned int rate)
{
    sample_rate = rate;
}

} // namespace
