/*  aKode Cross-fader

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
#include "crossfader.h"

namespace aKode {

CrossFader::CrossFader(unsigned int time) : time(time),pos(0) {}

// T is the input/output type, S is the fast arithmetics type, Div is a division method
template<typename T, typename S, template<typename S> class Arithm>
static bool _doFrame(AudioFrame* in, int& pos, AudioFrame* frame)
{
    T** indata1 = (T**)in->data;
    T** indata2 = (T**)frame->data;
    //T** outdata = (T**)in->data;

    long length;
    long max = frame->length;
    if (pos >= max) return false;
    if (in->channels != frame->channels) return false;
    if (in->sample_width != frame->sample_width) return false;

    if (in->length > max-pos)
        length = in->length = max-pos;
    else
        length = in->length;

    S orgw; // weight of original
    S neww; // weight of new
    for(int j=0; j<length && pos<max; j++,pos++) {
        neww = pos;
        orgw = max-pos;

        for(int i=0; i<in->channels; i++) {
            S signal = 0;
            S remainder = 0;
            signal    += Arithm<S>::div(indata1[i][j],max)*neww;
            remainder += Arithm<S>::rem(indata1[i][j],max)*neww;
            signal    += Arithm<S>::div(indata2[i][pos],max)*orgw;
            remainder += Arithm<S>::rem(indata2[i][pos],max)*orgw;

            indata1[i][j] = (T)(signal+Arithm<S>::div(remainder,max));
        }
    }
    return true;
}

// T is the input/output type, S is the fast arithmetics type, Arithm defines devisions
template<typename T, typename S, template<typename S> class Arithm>
static bool _readFrame(AudioFrame* in, int& pos, AudioFrame* frame)
{
    T** indata = (T**)frame->data;
    T** outdata = (T**)in->data;

    S length;
    S max = frame->length;
    if (max-pos <= 1024)
        length = max-pos;
    else
        length = 1024;

    if (pos >= max) return false;
    in->reserveSpace(frame, (long)length);

    S weight;
    for(int j=0; j<length && pos<max; j++,pos++) {
        weight = (max-pos);

        for(int i=0; i<in->channels; i++) {
            S signal    = Arithm<S>::div(indata[i][pos],max)*weight;
            S remainder = Arithm<S>::rem(indata[i][pos],max)*weight;

            outdata[i][j] = (T)(signal+Arithm<S>::div(remainder,max));
        }
    }
    return true;
}

template<typename T>
static void _writeFrame(AudioFrame* in, AudioFrame* source)
{
    T** indata = (T**)in->data;
    T** outdata = (T**)source->data;

    int tpos = source->length;
    for (int i=0; i<in->channels; i++) {
        tpos = source->length;
        for(int j = 0; j<in->length && tpos < source->max; j++,tpos++)
        {
            outdata[i][tpos] = indata[i][j];
        }
    }
    source->length = tpos;
}

// This codes abuses the internal working of AudioFrame, and will need an
// update if AudioFrame is changed.
bool CrossFader::writeFrame(AudioFrame* in)
{
    if (source.max==0) {
        long length = (in->sample_rate*time)/1000;
        source.reserveSpace(in, length);
        source.length = 0;
    }
    if (source.length >= source.max) return false;
    if (in->sample_width < -32) {
        _writeFrame<double>(in, &source);
    } else
    if (in->sample_width < 0) {
        _writeFrame<float>(in, &source);
    } else
    if (in->sample_width <= 8) {
        _writeFrame<int8_t>(in, &source);
    } else
    if (in->sample_width <= 16) {
        _writeFrame<int16_t>(in, &source);
    } else
        _writeFrame<int32_t>(in, &source);
    return true;
}

bool CrossFader::doFrame(AudioFrame* in)
{
    if (in->sample_width < -32) {
        return _doFrame<double, double, Arithm_FP>(in, pos, &source);
    } else
    if (in->sample_width < 0) {
        return _doFrame<float,  float,  Arithm_FP>(in, pos, &source);
    } else
    if (in->sample_width <= 8) {
        return _doFrame<int8_t, int32_t, Arithm_Int>(in, pos, &source);
    } else
    if (in->sample_width <= 16) {
        return _doFrame<int16_t, int32_t, Arithm_Int>(in, pos, &source);
    } else
    if (in->sample_width <= 24) {
        return _doFrame<int32_t, int32_t, Arithm_Int>(in, pos, &source);
    } else
        return _doFrame<int32_t, int64_t, Arithm_Int>(in, pos, &source);
}

bool CrossFader::readFrame(AudioFrame* in)
{
    if (in->sample_width < -32) {
        return _readFrame<double, double, Arithm_FP>(in, pos, &source);
    } else
    if (in->sample_width < 0) {
        return _readFrame<float,  float,  Arithm_FP>(in, pos, &source);
    } else
    if (in->sample_width <= 8) {
        return _readFrame<int8_t, int32_t, Arithm_Int>(in, pos, &source);
    } else
    if (in->sample_width <= 16) {
        return _readFrame<int16_t, int32_t, Arithm_Int>(in, pos, &source);
    } else
    if (in->sample_width <= 24) {
        return _readFrame<int32_t, int32_t, Arithm_Int>(in, pos, &source);
    } else
        return _readFrame<int32_t, int64_t, Arithm_Int>(in, pos, &source);
}

void CrossFader::setLength(unsigned int new_time) {
    time = new_time;
}

bool CrossFader::full() {
    return source.max <= source.length;
}

bool CrossFader::done() {
    return source.max > 0 && pos >= source.length;
}


} // namespace
