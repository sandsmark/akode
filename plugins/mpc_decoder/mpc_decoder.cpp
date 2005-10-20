/*  aKode: Musepack(MPC) Decoder

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

#include "config.h"
//#ifdef AKODE_COMPILE_MPC

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#else
#include <stdint.h>
#endif

#include "file.h"
#include "audioframe.h"

#include "mpc_dec.h"

#include "mpc_decoder.h"

namespace aKode {

class MPC_reader_impl : public MPC_reader
{
public:
	MPC_reader_impl(aKode::File *file, bool p_seekable) : m_file(file), m_seekable(p_seekable)
	{
            m_file->openRO();
            m_file->fadvise();
	}
        virtual ~MPC_reader_impl() {
            m_file->close();
        }
	size_t read ( void *ptr, size_t size ) {return m_file->read((char*)ptr,size);}
	bool seek ( int offset, int origin ) {return m_file->seek(offset,origin);}
	long tell () {return m_file->position();}
	long get_size () {return m_file->length();}
	bool canseek() {return m_file->seekable();}
private:
	aKode::File * m_file;
	bool m_seekable;
};

bool MPCDecoderPlugin::canDecode(File* src) {
    MPC_reader_impl reader(src, true);
    StreamInfo si;

    int r = si.ReadStreamInfo(&reader);
    return r==0;
}

extern "C" { MPCDecoderPlugin mpc_decoder; }

struct MPCDecoder::private_data
{
    private_data(File *src) : reader(src,true), decoder(&reader),
                              initialized(false), buffer(0), position(0),
                              eof(false), error(false)
    {};
    MPC_reader_impl reader;
    StreamInfo si;
    MPC_decoder decoder;

    bool initialized;
    MPC_SAMPLE_FORMAT *buffer;

    long position;
    bool eof;
    bool error;

    AudioConfiguration config;
};

MPCDecoder::MPCDecoder(File *src) {
    m_data = new private_data(src);
}

MPCDecoder::~MPCDecoder() {
    if (m_data->initialized)
        delete[] m_data->buffer;
    delete m_data;
}

void MPCDecoder::initialize() {
    if (m_data->initialized) return;

    m_data->si.ReadStreamInfo(&m_data->reader);
    m_data->error = !m_data->decoder.Initialize(&m_data->si);
    m_data->buffer = new MPC_SAMPLE_FORMAT[MPC_decoder::DecodeBufferLength];
    m_data->initialized = true;

    m_data->config.channels = m_data->si.simple.Channels;
    m_data->config.sample_rate = m_data->si.simple.SampleFreq;
    m_data->config.sample_width = -32;

    if (m_data->config.channels <=2)
        m_data->config.channel_config = MonoStereo;
    else
        m_data->config.channel_config = MultiChannel;

}
/*
template<int bits>
static inline int32_t scale(float sample)
{
  static const int32_t max = (1<<(bits-1))-1;

  // scale
  sample *= max;

  // round
  sample += 0.5;

  // clip
  if (sample > max)
    sample = max;
  else
  if (sample < -max)
    sample = -max;

  return (int32_t)sample;
}*/

bool MPCDecoder::readFrame(AudioFrame* frame) {
    if (!m_data->initialized) initialize();

    int status = m_data->decoder.Decode(m_data->buffer);

    if (status == -1) {
        m_data->error=true;
        return false;
    }
    if (status == 0) {
        m_data->eof=true;
        return false;
    }

    int channels = m_data->config.channels;
    long length = status;
    frame->reserveSpace(&m_data->config, length);
    m_data->position+=length;

    // Demux into frame
    /*
    int16_t** data = (int16_t**)frame->data;
    for(int i=0; i<length; i++)
        for(int j=0; j<channels; j++) {
            data[j][i] = scale<16>(m_data->buffer[i*channels+j]);
        }*/

    float** data = (float**)frame->data;
    for(int i=0; i<length; i++)
        for(int j=0; j<channels; j++)
            data[j][i] = m_data->buffer[i*channels+j];

    frame->pos = position();
    return true;
}

long MPCDecoder::length() {
    if (!m_data->initialized) return -1;
    return (long)(m_data->si.GetLength()*1000.0);
}

long MPCDecoder::position() {
    if (!m_data->initialized) return -1;
    float mpcpos = ((float)m_data->position)/(float)m_data->si.simple.SampleFreq;
    return (long)(mpcpos*1000.0);
}

bool MPCDecoder::eof() {
    return m_data->eof;
}

bool MPCDecoder::error() {
    return m_data->error;
}

bool MPCDecoder::seekable() {
    return m_data->reader.canseek();
}

bool MPCDecoder::seek(long pos) {
    if (!m_data->initialized) return false;

    long samplepos = (long)((pos*(float)m_data->si.simple.SampleFreq)/1000.0);

    if (m_data->decoder.SeekSample(samplepos)) {
        m_data->position = samplepos;
        return true;
    }
    else
        return false;
}

const AudioConfiguration* MPCDecoder::audioConfiguration() {
    return &m_data->config;
}

} // namespace

//#endif
