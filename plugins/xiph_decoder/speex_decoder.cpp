/*  aKode: Speex-Decoder

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

#include "akodelib.h"

#ifdef HAVE_SPEEX

extern "C" {
#include <string.h>
#include <stdlib.h>
#include <speex.h>
#include <speex_header.h>
#include <speex_callbacks.h>
#include <speex_stereo.h>
#include <ogg/ogg.h>
}

#ifdef SPEEX_DEBUG
#include <iostream>
using std::cerr;
#endif

#include "file.h"
#include "audioframe.h"
#include "decoder.h"
#include "speex_decoder.h"

namespace aKode {

bool SpeexDecoderPlugin::canDecode(File* src) {
    char header[36];
    bool res = false;
    src->openRO();
    if (src->read(header, 36) == 36)
        if (memcmp(header, "OggS",4) == 0 )
            if (memcmp(header+28, "Speex   ",8) == 0) res = true;
    src->close();
    return res;
};

extern "C" { SpeexDecoderPlugin speex_decoder; };

struct SpeexDecoder::private_data
{
    SpeexBits bits;
    SpeexMode *mode;
    SpeexStereoState stereo;

    ogg_sync_state sync;
    ogg_stream_state stream;
    ogg_page page;        // current page
    ogg_packet packet;    // current packet

    void *dec_state;
    File *src;
#ifdef HAVE_SPEEX11
    int16_t *out_buffer;
#else
    float *out_buffer;
#endif

    unsigned int bitrate;
    int frame_size;
    int frames_per_packet;
    int frame_nr;
    AudioConfiguration config;
    int serialno;
    long position;
    bool seeked;

    bool initialized;
    bool error, eof;
};

SpeexDecoder::SpeexDecoder(File *src) {
    m_data = new private_data;
    m_data->src = src;
    m_data->out_buffer = 0;

    ogg_sync_init(&m_data->sync);

    m_data->dec_state = 0;
    SpeexStereoState initstereo = SPEEX_STEREO_STATE_INIT;
    m_data->stereo = initstereo;

    m_data->initialized = m_data->eof = m_data->error = false;
    m_data->frame_nr = 100000;
    m_data->position = 0;
    m_data->seeked = false;

    src->openRO();
    src->fadvise();
};

SpeexDecoder::~SpeexDecoder() {
    if (m_data->initialized) {
        speex_bits_reset(&m_data->bits);
        ogg_sync_clear(&m_data->sync);
        ogg_stream_clear(&m_data->stream);
        if (m_data->dec_state) speex_decoder_destroy(m_data->dec_state);
        m_data->src->close();
        delete[] m_data->out_buffer;
    }
    delete m_data;
}

bool SpeexDecoder::openFile() {
    m_data->error = false;
    while(ogg_sync_pageout(&m_data->sync, &m_data->page) != 1) {
        char *buf = ogg_sync_buffer(&m_data->sync, 1024);
        int read = m_data->src->read(buf, 1024);
        if (read <= 0) {
            m_data->error = true;
            return false;
        }
        ogg_sync_wrote(&m_data->sync, read);
    }
    m_data->serialno = ogg_page_serialno(&m_data->page);
    ogg_stream_init(&m_data->stream, m_data->serialno);
    speex_bits_init(&m_data->bits);

//    ogg_stream_pagein(&m_data->stream, &m_data->page);
    //ogg_stream_packetout(&m_data->stream, &m_data->packet);
//    ogg_stream_packetout(&m_data->stream, &m_data->packet);

    if(!decodeHeader())
    {
        m_data->error = true;
        return false;
    }

    m_data->initialized = true;
    return true;
}

bool SpeexDecoder::readPage() {

    while (ogg_sync_pageout(&m_data->sync, &m_data->page) != 1) {
        char *buf = ogg_sync_buffer(&m_data->sync, 4096);
        long read = m_data->src->read(buf, 4096);
        if (read <= 0) return false;
        ogg_sync_wrote(&m_data->sync, read);
    }

    ogg_stream_pagein(&m_data->stream, &m_data->page);
//    m_data->packets = ogg_page_packets(&m_data->page);
    return true;
}

bool SpeexDecoder::readPacket() {
    bool res = true;
    while (ogg_stream_packetpeek(&m_data->stream, &m_data->packet) != 1 && res) {
        res = readPage();
    }
    ogg_stream_packetout(&m_data->stream, &m_data->packet);
    speex_bits_read_from(&m_data->bits, (char*)m_data->packet.packet, m_data->packet.bytes);
    m_data->frame_nr = 0;

    return res;
}

bool SpeexDecoder::decodeHeader() {

    SpeexHeader *header;
    header = speex_packet_to_header((char*)m_data->page.body, m_data->page.body_len);
    if (!header) {
        // invalid file
        m_data->error = true;
	#ifdef SPEEX_DEBUG
        std::cerr << "Invalid file\n";
	#endif
        return false;
    }

    SpeexMode *mode = ( SpeexMode* )speex_mode_list[ header->mode ];
    m_data->mode = mode;
    m_data->config.channels = header->nb_channels;
    m_data->config.channel_config = MonoStereo;
    m_data->frames_per_packet = header->frames_per_packet;

    if (mode->bitstream_version != header->mode_bitstream_version) {
        // incompatible bitstream
        m_data->error = true;
        return false;
    }

    m_data->dec_state = speex_decoder_init(mode);
    speex_decoder_ctl(m_data->dec_state, SPEEX_GET_FRAME_SIZE, &m_data->frame_size);
    //m_data->bitrate = header->bitrate;
    speex_decoder_ctl(m_data->dec_state, SPEEX_GET_BITRATE, &m_data->bitrate);
    m_data->config.sample_rate = header->rate;
    m_data->config.sample_width = 16;
    speex_decoder_ctl(m_data->dec_state, SPEEX_SET_SAMPLING_RATE, &m_data->config.sample_rate);
    //speex_decoder_ctl(m_data->dec_state, SPEEX_GET_SAMPLING_RATE, &m_data->sample_rate);

    // Use the perceptial enhancement, which gives a subjectively better result
    // but is technically further from the source.
    int i = 1;
    speex_decoder_ctl(m_data->dec_state, SPEEX_SET_ENH, &i);

    // Handle the patched-on stereo stuff
    if (m_data->config.channels != 1) {
        SpeexCallback callback;
        callback.callback_id = SPEEX_INBAND_STEREO;
        callback.func = speex_std_stereo_request_handler;
        callback.data = &m_data->stereo;
        speex_decoder_ctl(m_data->dec_state, SPEEX_SET_HANDLER, &callback);
    }
    #ifdef HAVE_SPEEX11
    m_data->out_buffer = new int16_t[m_data->frame_size*m_data->config.channels];
    #else
    m_data->out_buffer = new float[m_data->frame_size*m_data->config.channels];
    #endif

    free(header);
    return true;
}

bool SpeexDecoder::readFrame(AudioFrame* frame)
{
    if (!m_data->initialized) openFile();

    if (m_data->eof || m_data->error) return false;

    if (m_data->frame_nr >= m_data->frames_per_packet) {
        if (!readPacket()) {
           m_data->eof = true;
           return false;
        }
    }
    #if defined(HAVE_SPEEX11) && !defined(BROKEN_SPEEX11)
    speex_decode_int(m_data->dec_state, &m_data->bits, m_data->out_buffer);
    #else
    speex_decode(m_data->dec_state, &m_data->bits, m_data->out_buffer);
    #endif

    int channels = m_data->config.channels;
    int length = m_data->frame_size;
    frame->reserveSpace(&m_data->config, length);

    if (m_data->config.channels == 2)
        #if defined(HAVE_SPEEX11) && !defined(BROKEN_SPEEX11)
        speex_decode_stereo_int(m_data->out_buffer, length, &m_data->stereo);
        #else
        speex_decode_stereo(m_data->out_buffer, length, &m_data->stereo);
        #endif


    for (int i=0; i<m_data->frame_size*m_data->config.channels; i++) {
        if (m_data->out_buffer[i] > 32766) m_data->out_buffer[i] = 32767;
        else
        if (m_data->out_buffer[i] < -32767) m_data->out_buffer[i] = -32768;
        else
            m_data->out_buffer[i] = m_data->out_buffer[i];
    };

    // Decode into frame
    int16_t** data = (int16_t**)frame->data;
    for(int i=0; i<length; i++)
        for(int j=0; j<channels; j++)
            #if defined(HAVE_SPEEX11)
            data[j][i] = m_data->out_buffer[i*channels+j];
            #else
            data[j][i] = (int16_t)(m_data->out_buffer[i*channels+j]+0.5);
            #endif


    m_data->position += m_data->frame_size;
    frame->pos = position();
    m_data->frame_nr++;
    return true;
}

long SpeexDecoder::length() {
    if (!m_data->bitrate || !m_data->initialized) return -1;
    float spxlen = (8.0*m_data->src->length())/(float)m_data->bitrate;
    return (long)(spxlen*1000.0);
}

long SpeexDecoder::position() {
    if (!m_data->bitrate || !m_data->initialized) return -1;
    float spxpos = ((float)m_data->position)/(float)m_data->config.sample_rate;

    if (m_data->seeked) {
        float tellpos = (8.0*m_data->src->position())/(float)m_data->bitrate;
        // tellpos should always be somewhat ahead, if spxpos is worse use tellpos
        if (tellpos < spxpos) {
            spxpos = tellpos;
            m_data->position = (long)(tellpos*m_data->config.sample_rate);
        }
    }

    return (long)(spxpos*1000.0);
}

bool SpeexDecoder::eof() {
    return m_data->eof || m_data->error;
     /*return m_data->error || (m_data->src->eof() && m_data->frame_nr >= m_data->frames_per_packet); */
}

bool SpeexDecoder::error() {
    return m_data->error;
}

bool SpeexDecoder::seekable() {
    return m_data->src->seekable();
}

bool SpeexDecoder::seek(long pos) {
    if(!m_data->initialized) return false;

    long bpos = (long)(((float)pos*(float)m_data->bitrate)/8000.0);
    if (m_data->src->seek(bpos)) {
        speex_bits_reset(&m_data->bits);
        ogg_sync_reset(&m_data->sync);
        ogg_stream_reset(&m_data->stream);
        readPage();
        readPacket();
        // We should now have read in a whole new page
        bpos = (m_data->src->position()-m_data->page.body_len);
        m_data->position = (long)((bpos*8.0*m_data->config.sample_rate)/(float)m_data->bitrate);
        m_data->seeked = true;
        return true;
    }
    return false;
}

const AudioConfiguration* SpeexDecoder::audioConfiguration() {
    return &m_data->config;
}

} // namespace

#endif // HAVE_SPEEX
