/*  aKode: Wav-Decoder

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

#include <string.h>

#include "audioframe.h"
#include "decoder.h"
#include "file.h"
#include "wav_decoder.h"

#include <iostream>

namespace aKode {

bool WavDecoderPlugin::canDecode(File* src) {
    char header[4];
    bool res = false;
    src->openRO();
    if (src->read(header, 4) != 4 || memcmp(header, "RIFF",4) != 0 ) goto close;
    src->seek(8);
    if (src->read(header, 4) != 4 || memcmp(header, "WAVE",4) != 0 ) goto close;
    src->seek(20);
    if (src->read(header, 2) != 2 || memcmp(header, "\x01\0",2) != 0 ) goto close;
    res = true;
close:
    src->close();
    return res;

}

extern "C" { WavDecoderPlugin wav_decoder; }

struct WavDecoder::private_data
{
    private_data() : valid(false), eof(false), buffer_length(0), buffer(0) {};
    AudioConfiguration config;
    bool valid;
    bool eof;
    long position;

    long pos;
    long length;

    unsigned int buffer_length;
    unsigned char *buffer;

    File *src;
};


WavDecoder::WavDecoder(File *src) {
    d = new private_data;

    openFile(src);
}

bool WavDecoder::openFile(File* src) {
    d->src = src;
    src->openRO();
    src->fadvise();

    // Get format information
    unsigned char buffer[4];
    src->seek(4);
    src->read((char*)buffer, 4); // size of stream
    d->length = buffer[0] + buffer[1]*256 + buffer[2] * (1<<16) + buffer[3] * (1<<24) + 8;

    src->seek(16);
    src->read((char*)buffer, 4); // should really be 16
    d->pos = 20 + buffer[0] + buffer[1]*256;
    if (buffer[2] != 0 || buffer[3] != 0) goto invalid;

    src->read((char*)buffer, 2); // check for compression
    if (*(short*)buffer != 1) goto invalid;

    src->read((char*)buffer, 2);
    d->config.channels = buffer[0] + buffer[1]*256;
    if (d->config.channels <=2)
        d->config.channel_config = MonoStereo;
    else
        d->config.channel_config = MultiChannel;

    src->read((char*)buffer, 4);
    d->config.sample_rate = buffer[0] + buffer[1]*256 + buffer[2] * (1<<16) + buffer[3] * (1<<24);

    src->seek(34);
    src->read((char*)buffer, 2);
    d->config.sample_width = buffer[0] + buffer[1]*256;

    // Various sanity checks
    if (d->config.sample_width != 8 && d->config.sample_width != 16 && d->config.sample_width != 32) goto invalid;
    if (d->config.sample_rate > 200000) goto invalid;

find_data:
    src->seek(d->pos);
    src->read((char*)buffer, 4);
    if (memcmp(buffer, "data", 4) != 0)
      if (memcmp(buffer, "clm ", 4) != 0)
        goto invalid;
      else {
        src->read((char*)buffer, 4);
        d->pos = d->pos+ 8 + buffer[0] + buffer[1]*256;
        goto find_data;
      }

    src->seek(d->pos+8); // start of data
    d->position = 0;
    d->valid = true;
    d->eof = false;
    // 1024 samples:
    d->buffer_length = 1024*((d->config.sample_width+7)/8)*d->config.channels;
    d->buffer = new unsigned char[d->buffer_length];
    return true;

invalid:
    std::cerr << "Invalid WAV file\n";
    d->valid = false;
    src->close();
    return false;
}

void WavDecoder::close() {
    d->src->close();
    delete[] d->buffer;
    d->valid = false;
}

WavDecoder::~WavDecoder() {
    delete d;
}

bool WavDecoder::readFrame(AudioFrame* frame)
{
    if (!d->valid || d->eof) return false;

    unsigned long samples = 1024;
    // read a frame
    unsigned long length;
    length = d->src->read((char*)d->buffer, d->buffer_length);
    if (length != d->buffer_length) {
        samples = length / (d->config.channels * ((d->config.sample_width+7)/8));
        if (d->src->eof()) d->eof = true;
    }
    d->pos += length;
    d->position += samples;
    // Ensure the frame is properly configured
    frame->reserveSpace(&d->config, samples);

    int channels = d->config.channels;
    if (d->config.sample_width == 8) {
        // WAV 8bit is unsigned
        uint8_t* buffer = (uint8_t*)d->buffer;
        int8_t** data = (int8_t**)frame->data;
        for(unsigned int i=0; i<samples; i++)
            for(int j=0; j<channels; j++)
                data[j][i] = (int(buffer[i*channels+j])) - 128;
    }
    else
    if (d->config.sample_width == 16) {
        int16_t* buffer = (int16_t*)d->buffer;
        int16_t** data = (int16_t**)frame->data;
        for(unsigned int i=0; i<samples; i++)
            for(int j=0; j<channels; j++)
                data[j][i] = buffer[i*channels+j];
    } else
    if (d->config.sample_width == 32) {
        int32_t* buffer = (int32_t*)d->buffer;
        int32_t** data = (int32_t**)frame->data;
        for(unsigned int i=0; i<samples; i++)
            for(int j=0; j<channels; j++)
                data[j][i] = buffer[i*channels+j];
    } else
        return false;

    frame->pos = position();

    return true;
}

long WavDecoder::length() {
    if (!d->valid) return -1;
    long byterate = d->config.sample_rate * d->config.channels * ((d->config.sample_width+7)/8);
    return (d->length-44)/byterate;
}

long WavDecoder::position() {
    if (!d->valid) return -1;
    long div = (d->position / d->config.sample_rate) * 1000;
    long rem = (d->position % d->config.sample_rate) * 1000;
    return div + (rem / d->config.sample_rate);
}

bool WavDecoder::eof() {
    return d->eof;
}

bool WavDecoder::error() {
    return !d->valid;
}

bool WavDecoder::seek(long pos) {
    int samplesize = d->config.channels * ((d->config.sample_width+7)/8);
    long byterate = d->config.sample_rate * samplesize;
    long sample_pos = (pos * byterate) / 1000;
    long byte_pos = sample_pos * samplesize;
    if (byte_pos+44 >= d->length) return false;
    if (!d->src->seek(byte_pos+44)) return false;
    d->pos = byte_pos + 44;
    return true;
}

bool WavDecoder::seekable() {
    return d->src->seekable();
}

const AudioConfiguration* WavDecoder::audioConfiguration() {
    if (!d->valid) return 0;
    return &d->config;
}

} // namespace
