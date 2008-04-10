/*  aKode: Converter

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

#include "arithmetic.h"
#include "audioframe.h"
#include "converter.h"

namespace aKode {

Converter::Converter(int sample_width) : m_sample_width(sample_width) {}

template<typename T, typename S, template<typename T> class ArithmT, template<typename S> class ArithmS>
static bool __doFrameFP(AudioFrame* in, AudioFrame* out, int sample_width)
{
    if (out)
        out->reserveSpace(in, in->length);
    else
        out = in;

    // ### Use doubles if double sized or 32bit samples are used.
    float scale = 1.0;
    scale = ArithmT<T>::max(in->sample_width)/(float)ArithmS<S>::max(sample_width);

    int channels = in->channels;
    int length = in->length;

    T** indata = (T**)in->data;
    S** outdata = (S**)out->data;
    for(int i=0; i<channels; i++)
        for(int j=0; j<length; j++)
            outdata[i][j] = (S)(scale*indata[i][j]);

    out->sample_width = sample_width;
    return true;
}

template<typename T, typename S>
static bool __doFrame(AudioFrame* in, AudioFrame* out, int sample_width)
{
    if (out)
        out->reserveSpace(in, in->length);
    else
        out = in;

    int width_T = sizeof(T)*8;
    int shift = width_T - sample_width;

    int channels = in->channels;
    uint32_t length = in->length;

    T** indata = (T**)in->data;
    S** outdata = (S**)out->data;
    for(int i=0; i<channels; i++)
        for(uint32_t j=0; j<length; j++)
            outdata[i][j] = (S)(indata[i][j] >> shift);

    out->sample_width = sample_width;
    return true;
}

template<typename T>
static inline bool _doFrameFP(AudioFrame* in, AudioFrame* out, int sample_width)
{
    if (in->sample_width < 0) {
        return __doFrameFP<float, T, Arithm_FP, Arithm_FP>(in, out, sample_width);
    } else
    if (in->sample_width <= 8) {
        return __doFrameFP<int8_t, T, Arithm_Int, Arithm_FP>(in, out, sample_width);
    } else
    if (in->sample_width <= 16) {
        return __doFrameFP<int16_t, T, Arithm_Int, Arithm_FP>(in, out, sample_width);
    } else
        return __doFrameFP<int32_t, T, Arithm_Int, Arithm_FP>(in, out, sample_width);

}

template<typename T>
static inline bool _doFrame(AudioFrame* in, AudioFrame* out, int sample_width)
{
    if (in->sample_width < 0) {
        return __doFrameFP<float, T, Arithm_FP, Arithm_Int>(in, out, sample_width);
    } else
    if (in->sample_width <= 8) {
        return __doFrame<int8_t, T>(in, out, sample_width);
    } else
    if (in->sample_width <= 16) {
        return __doFrame<int16_t, T>(in, out, sample_width);
    } else
        return __doFrame<int32_t, T>(in, out, sample_width);
}

bool Converter::doFrame(AudioFrame* in, AudioFrame* out)
{
    if (m_sample_width == 0) return false;
    if (!out && in->sample_width == m_sample_width) return true;

    if (m_sample_width < 0) {
        return _doFrameFP<float>(in, out, m_sample_width);
    } else
    if (m_sample_width <= 8) {
        return _doFrame<int8_t>(in, out, m_sample_width);
    } else
    if (m_sample_width <= 16) {
        return _doFrame<int16_t>(in, out, m_sample_width);
    } else
        return _doFrame<int32_t>(in, out, m_sample_width);
}

void Converter::setSampleWidth(int sample_width)
{
    m_sample_width = sample_width;
}


} // namespace
