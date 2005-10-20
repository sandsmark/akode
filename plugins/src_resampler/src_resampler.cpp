/*  aKode Resampler (Secret Rabbit Code)

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

#include "akodelib.h"

#include <samplerate.h>

#include "audioframe.h"
#include "src_resampler.h"

namespace aKode {

SRCResampler* SRCResamplerPlugin::openResampler() {
    return new SRCResampler();
}

extern "C" { SRCResamplerPlugin src_resampler; };

SRCResampler::SRCResampler() : sample_rate(44100) {};

static void _convert1(AudioFrame *in, float* outdata)
{
    long length = in->length;
    int channels = in->channels;

    float** indata = (float**)in->data;
    for(int j=0; j<length; j++) {
        for(int i=0; i<channels; i++) {
            outdata[i+j*channels] = indata[i][j];
        }
    }
}

template<typename S>
static void _convert1(AudioFrame *in, float* outdata)
{
    float scale = (float)(1<<(in->sample_width-1));
    scale = 1.0/scale;

    int channels = in->channels;

    S** indata = (S**)in->data;
    for(int j=0; j<in->length; j++) {
        for(int i=0; i<channels; i++) {
            outdata[i+j*channels] = scale*indata[i][j];
        }
    }
}

static void _convert2(float *indata, AudioFrame* out)
{
    long length = out->length;
    int channels = out->channels;

    float** outdata = (float**)out->data;
    for(int j=0; j<length; j++) {
        for(int i=0; i<channels; i++) {
            outdata[i][j] = indata[i+j*channels];
        }
    }
}

template<typename S>
static void _convert2(float *indata, AudioFrame* out)
{
    float scale = (float)(1<<(out->sample_width-1));

    int channels = out->channels;

    S** outdata = (S**)out->data;
    for(int j=0; j<out->length; j++) {
        for(int i=0; i<channels; i++) {
            outdata[i][j] = (S)(scale*indata[i+j*channels]);
        }
    }
}

// A high quality resampling by using libsamplerate
bool SRCResampler::doFrame(AudioFrame* in, AudioFrame* out)
{
    // Ouch, I dont like this much mallocing
    float *tmp1 = new float[in->channels*in->length];
    float *tmp2 = new float[in->channels*in->length];

    // convert to float frames
    if (in->sample_width < 0) {
        _convert1(in, tmp1);
    } else
    if (in->sample_width <= 8) {
        _convert1<int8_t>(in, tmp1);
    } else
    if (in->sample_width <= 16) {
        _convert1<int16_t>(in, tmp1);
    } else
        _convert1<int32_t>(in, tmp1);

    float newspeed = speed * (in->sample_rate/(float)sample_rate);
    long outlength = (long)(in->length*newspeed);
    out->reserveSpace(in->channels, outlength, in->sample_width);
    out->sample_rate = sample_rate;
    out->channel_config = in->channel_config;
    out->surround_config = in->surround_config;
    out->pos = in->pos;

    // We should figure out if interleaving first is faster
    // than making a call per channel
//    for (int i=0; i<in->channels; i++) {
        SRC_DATA src_data;

        src_data.data_in = tmp1;
        src_data.data_out = tmp2;
        src_data.input_frames = in->length;
        src_data.output_frames = out->length;
        src_data.src_ratio = newspeed;

        src_simple(&src_data, SRC_SINC_MEDIUM_QUALITY, in->channels);
//    }

    // convert from float frames
    if (out->sample_width > 0) {
        if (out->sample_width <= 8) {
            _convert2<int8_t>(tmp2, out);
        } else
        if (out->sample_width <= 16) {
            _convert2<int16_t>(tmp2, out);
        } else
            _convert2<int32_t>(tmp2, out);
    } else
        _convert2(tmp2, out);


    delete[] tmp1;
    delete[] tmp2;

    return true;
}

void SRCResampler::setSpeed(float value) {
    speed = value;
}

void SRCResampler::setSampleRate(unsigned int rate) {
    sample_rate = rate;
}

} // namespace
