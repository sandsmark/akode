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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <stdio.h>
#include <string.h>

#include "audioframe.h"
#include "decoder.h"
#include "file.h"
#include "wav_decoder.h"

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
    private_data() : buffer_length(0), buffer(0) {};
    AudioConfiguration config;
    bool valid;

    long pos;
    long length;

    unsigned int buffer_length;
    unsigned char *buffer;

    File *src;
};


WavDecoder::WavDecoder(File *src) {
    m_data = new private_data;

    openFile(src);
}

bool WavDecoder::openFile(File* src) {
    m_data->src = src;
    src->openRO();
    src->fadvise();

    // Get format information
    unsigned char buffer[4];
    src->seek(4);
    src->read((char*)buffer, 4); // size of stream
    m_data->length = buffer[0] + buffer[1]*256 + buffer[2] * (1<<16) + buffer[3] * (1<<24) + 8;

    src->seek(16);
    src->read((char*)buffer, 4); // should really be 16
    m_data->pos = 20 + buffer[0] + buffer[1]*256;
    if (buffer[2] != 0 || buffer[3] != 0) goto invalid;

    src->read((char*)buffer, 2); // check for compression
    if (*(short*)buffer != 1) goto invalid;

    src->read((char*)buffer, 2);
    m_data->config.channels = buffer[0] + buffer[1]*256;
    if (m_data->config.channels <=2)
        m_data->config.channel_config = MonoStereo;
    else
        m_data->config.channel_config = MultiChannel;

    src->read((char*)buffer, 4);
    m_data->config.sample_rate = buffer[0] + buffer[1]*256 + buffer[2] * (1<<16) + buffer[3] * (1<<24);

    src->seek(34);
    src->read((char*)buffer, 2);
    m_data->config.sample_width = buffer[0] + buffer[1]*256;

    // Various sanity checks
    if (m_data->config.sample_width != 8 && m_data->config.sample_width != 16 && m_data->config.sample_width != 32) goto invalid;
    if (m_data->config.sample_rate > 200000) goto invalid;

find_data:
    src->seek(m_data->pos);
    src->read((char*)buffer, 4);
    if (memcmp(buffer, "data", 4) != 0)
      if (memcmp(buffer, "clm ", 4) != 0)
        goto invalid;
      else {
        src->read((char*)buffer, 4);
        m_data->pos = m_data->pos+ 8 + buffer[0] + buffer[1]*256;
        goto find_data;
      }

    src->seek(m_data->pos+8); // start of data
    m_data->valid = true;
    m_data->buffer_length = 4096*((m_data->config.sample_width+7)/8)*m_data->config.channels; // 4096 samples
    m_data->buffer = new unsigned char[m_data->buffer_length];
    return true;

invalid:
    m_data->valid = false;
    src->close();
    return false;
}

WavDecoder::~WavDecoder() {
    m_data->src->close();
    delete[] m_data->buffer;
    delete m_data;
}

bool WavDecoder::readFrame(AudioFrame* frame)
{
    if (!m_data->valid) return false;

    unsigned long samples = 4096;
    // read a frame
    unsigned long length;
    length = m_data->src->read((char*)m_data->buffer, m_data->buffer_length);
    if (length != m_data->buffer_length) {
        samples = length / (m_data->config.channels * ((m_data->config.sample_width+7)/8));
        if (m_data->src->eof()) {
            m_data->src->close();
            m_data->valid = false;
        }
    }

    // Ensure the frame is properly configured
    frame->reserveSpace(&m_data->config, samples);

    int channels = m_data->config.channels;
    if (m_data->config.sample_width == 8) {
        uint8_t* buffer = (uint8_t*)m_data->buffer;
        int8_t** data = (int8_t**)frame->data;
        for(unsigned int i=0; i<samples; i++)
            for(int j=0; j<channels; j++)
                data[j][i] = (int)(buffer[i*channels+j])-128;
    }
    else
    if (m_data->config.sample_width == 16) {
        int16_t* buffer = (int16_t*)m_data->buffer;
        int16_t** data = (int16_t**)frame->data;
        for(unsigned int i=0; i<samples; i++)
            for(int j=0; j<channels; j++)
                data[j][i] = buffer[i*channels+j];
    } else
    if (m_data->config.sample_width == 32) {
        int32_t* buffer = (int32_t*)m_data->buffer;
        int32_t** data = (int32_t**)frame->data;
        for(unsigned int i=0; i<samples; i++)
            for(int j=0; j<channels; j++)
                data[j][i] = buffer[i*channels+j];
    } else
        return false;

    return true;
}

long WavDecoder::length() {
    if (!m_data->valid) return -1;
    long byterate = m_data->config.sample_rate * m_data->config.channels * ((m_data->config.sample_width+7)/8);
    return (m_data->length-44)/byterate;
}

long WavDecoder::position() {
    if (!m_data->valid) return -1;
    long byterate = m_data->config.sample_rate * m_data->config.channels * ((m_data->config.sample_width+7)/8);
    return (m_data->pos-44)/byterate;
}

bool WavDecoder::eof() {
    return (m_data->src->eof());
}

bool WavDecoder::error() {
    return !m_data->valid;
}

bool WavDecoder::seek(long pos) {
    int samplesize = m_data->config.channels * ((m_data->config.sample_width+7)/8);
    long byterate = m_data->config.sample_rate * samplesize;
    long sample_pos = (pos * byterate) / 1000;
    long byte_pos = sample_pos * samplesize;
    if (byte_pos+44 >= m_data->length) return false;
    if (!m_data->src->seek(byte_pos+44)) return false;
    m_data->pos = byte_pos + 44;
    return true;
}

bool WavDecoder::seekable() {
    return m_data->src->seekable();
}

const AudioConfiguration* WavDecoder::audioConfiguration() {
    if (!m_data->valid) return 0;
    return &m_data->config;
}

} // namespace
