/*  aKode: MPEG-Decoder (MAD-decoder)

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

// uncomment to debug the plugin
//#define MPEG_DEBUG

#include <string.h>
#include <mad.h>

#ifdef MPEG_DEBUG
#include <iostream>
using std::cerr;
#endif

#include "file.h"
#include "audioframe.h"
#include "framedecoder.h"
#include "mpeg_decoder.h"

namespace aKode {

bool MPEGDecoderPlugin::canDecode(File* src) {
    char header[6];
    unsigned char *buf = (unsigned char*)header; // C-stuff
    bool res = false;
    src->openRO();
    if(src->read(header, 4)) {
        // skip id3v2 headers
        if (memcmp(header, "ID3", 3) == 0) {
            src->read(header, 6);
            int size = 0;
            if (buf[1] & 0x10) size += 10;
            size += buf[5];
            size += buf[4] << 7;
            size += buf[3] << 14;
            size += buf[2] << 21;
            src->seek(10+size);
            src->read(header, 4);
        }
        if (buf[0] == 0xff && (buf[1] & 14)) // frame synchronizer
            if((buf[1] & 0x18) != 0x08) // support MPEG 1, 2 and 2½
                if((buf[1] & 0x06) != 0x00) // Layer I, II and III
                    res = true;
    }
    src->close();
    return res;
};

extern "C" { MPEGDecoderPlugin mpeg_decoder; };

struct xing_frame {
    // Xing header:
    bool has_toc;                 // valid table-of-contents
    unsigned int bytes;           // nr-of-bytes (0 if unknown)
    unsigned int frames;          // nr-of-frames (0 if unknown)
    unsigned char toc[100];       // time: x/100 -> place: y/256 (y = toc[x])
};

#define FILEBUFFER_SIZE 8192
struct MPEGDecoder::private_data
{
    private_data() : position(0),
                     length(0),
                     bitrate(0),
                     eof(false),
                     error(false),
                     initialized(false),
                     id3v2size(0),
                     xing_vbr(false),
                     unknown_vbr(false)
    {};
    mad_stream stream;
    mad_frame frame;
    mad_synth synth;

    File *src;
    AudioConfiguration config;

    long position, length;
    int bitrate, layer;

    bool eof, error, initialized;

    int id3v2size;               // size of prepended ID3v2 tag
    bool xing_vbr;               // Xing-compatible VBR
    bool unknown_vbr;            // Oh, bugger...
    xing_frame xing;

    char filebuffer[FILEBUFFER_SIZE];

    bool metaframe_filter(bool fast);
};

MPEGDecoder::MPEGDecoder(File *src) {
    m_data = new private_data();

    mad_stream_init(&m_data->stream);
    mad_frame_init(&m_data->frame);
    mad_synth_init(&m_data->synth);

    mad_stream_options(&m_data->stream, MAD_OPTION_IGNORECRC);

    m_data->eof = false;
    m_data->error = false;

    m_data->src = src;
    if (!m_data->src->openRO()) {
        m_data->error = true;
        #ifdef MPEG_DEBUG
        cerr << "Could not open file: " << m_data->src->filename << "\n";
        #endif
    }
    m_data->src->fadvise();

    m_data->length = m_data->src->length();
};

MPEGDecoder::~MPEGDecoder() {
    mad_stream_finish(&m_data->stream);
    mad_frame_finish(&m_data->frame);
    mad_synth_finish(&m_data->synth);
    m_data->src->close();
    delete m_data;
}

bool MPEGDecoder::skipID3v2() {
    m_data->src->seek(0);
    m_data->id3v2size = 0;
    char buf[16];
    if (m_data->src->read(buf, 10)) {
        // skip id3v2 headers
        if (memcmp(buf, "ID3", 3) == 0) {
            #ifdef MPEG_DEBUG
            cerr << "ID3v2 tag found\n";
            #endif
            int size = 0;
            if (buf[5] & 0x10) size += 10;
            size += buf[9];
            size += buf[8] << 7;
            size += buf[7] << 14;
            size += buf[6] << 21;
            m_data->id3v2size = size;
            if (!m_data->src->seek(10+size)) {
                char space[256];
                for(int i=0; i<size; ) {
                    i += m_data->src->read(space, (size-i) > 256 ?  256 : (size-i));
                }
            }
            return true;
        }
    }
    if (!m_data->src->seek(0)) {
        mad_stream_buffer(&m_data->stream, (uint8_t*)buf, 10);
    }
    return false;
}

bool MPEGDecoder::moreData(bool flush)
{
    mad_stream *stream = &m_data->stream;

    long rem = 0;
    if (stream->next_frame != 0 && !flush) {
        rem = stream->bufend - stream->next_frame;
        memmove(m_data->filebuffer, stream->next_frame, rem);
    }
    #ifdef MPEG_DEBUG
//    cerr << "remainder=" << rem << "\n";
    #endif

    int res = m_data->src->read(m_data->filebuffer+rem, FILEBUFFER_SIZE-rem);
    mad_stream_buffer(stream, (uint8_t*)m_data->filebuffer, res+rem);
    // for some reason MAD can't to do this itself:
    if (stream->error == MAD_ERROR_LOSTSYNC || flush) stream->sync = 0;
    stream->error = MAD_ERROR_NONE;

    if (res == 0) {
        m_data->eof = true;
        return false;
    } else
    if (res < 0) {
        m_data->error = true;
        return false;
    } else
        return true;
}

// originaly from minimad.c
template<int bits>
static inline int32_t scale(mad_fixed_t sample)
{
  /* round */
  sample += (1L << (MAD_F_FRACBITS - bits));

  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - bits);
}

static inline long postotime(long pos, int sample_rate) {
    if (sample_rate == 0) return 0;
    long sec = pos/sample_rate;
    long rem = pos%sample_rate;
    long ms = (rem*1000)/sample_rate;
    return sec*1000+ms;
}


// Xing-header documentation is from http://www.multiweb.cz/twoinches/MP3inside.htm
// toc-format verified in LAME source-code
static bool xing_decode(xing_frame *xing, const unsigned char * bytes)
{
    mad_bitptr ptr;
    mad_bit_init(&ptr, bytes);

    if (mad_bit_read(&ptr, 32) == (('X' << 24) | ('i' << 16) | ('n' << 8) | 'g'))
    {
        int flags = mad_bit_read(&ptr, 32);
        if (flags & 1)
            xing->frames = mad_bit_read(&ptr, 32);
        if (flags & 2)
            xing->bytes = mad_bit_read(&ptr, 32);
        if (flags & 4) {
            xing->has_toc = true;
            for (int i=0; i<100; i++) {
                xing->toc[i] = mad_bit_read(&ptr, 8);
            }
        }
        return true;
    }
    return false;
}

// Detect various metaframes
bool MPEGDecoder::private_data::metaframe_filter(bool fast)
{
    if (stream.anc_bitlen < 16) return false;

    const unsigned char* ptr;

    ptr = stream.anc_ptr.byte;
test:
    if ((ptr[0] == 'X') && (ptr[1] == 'i') && (ptr[2] == 'n') && (ptr[3] =='g'))
    {
        #ifdef MPEG_DEBUG
        cerr << "Xing(VBR) header found\n";
        #endif
        xing_vbr = true;
        xing_decode(&xing, ptr);
        return true;
    } /*
    if ((ptr[0] == 'L') || (ptr[1] == 'A') || (ptr[2] == 'M') || (ptr[3] =='E'))
    {
        #ifdef MPEG_DEBUG
        cerr << "LAME frame found\n";
        #endif
        return true;
    }*/
    if ((ptr[0] == 'I') && (ptr[1] == 'n') && (ptr[2] == 'f') && (ptr[3] =='o'))
    {
        #ifdef MPEG_DEBUG
        cerr << "Info(CBR) header found\n";
        #endif
        return true;
    }

    if (!fast) {
        // We can't trust broken-encoders to _not_ fuck this up
        ptr = stream.this_frame;
        // skip frame header and side info
        ptr += 6;
        // Search for metaframe tags:
        for(int i=0; i<64 && ptr < stream.bufend; i++) {
            if (*ptr == 'X' || *ptr== 'I') {
                fast = true;
                goto test;
            }
            ptr++;
        }
    }

    return false;
}

bool MPEGDecoder::sync()
{
    // only try 32 times, if no sync-frame is found the file is properbly not MPEG
    for(int i=0; i<32; i++)  {
        int res = mad_stream_sync(&m_data->stream);
        if (res == 0 ) goto got_sync;
        if (!moreData(true)) break;
    }
    m_data->error=true;
    #ifdef MPEG_DEBUG
    cerr << "Synchronization failed, likely not a mpeg audio file.\n";
    #endif
    return false;

got_sync:
    return true;
}

bool MPEGDecoder::prepare()
{
    skipID3v2();
    // initialize stream
    if (m_data->stream.buffer == 0) moreData();

    if (sync()) {
        m_data->initialized = true;
        return true;
    } else
        return false;
}

static void setChannelConfiguration(AudioConfiguration& config, mad_mode mode)
{
    switch(mode) {
        case MAD_MODE_SINGLE_CHANNEL:
            config.channels=1;
            config.channel_config=MonoStereo;
            break;
        case MAD_MODE_DUAL_CHANNEL:
            config.channels=2;
            config.channel_config=MultiChannel;
            break;
        default: // Stereo and joint stereo
            config.channels=2;
            config.channel_config=MonoStereo;
    }
    config.surround_config = 0;
}

bool MPEGDecoder::readFrame(AudioFrame* frame)
{
    if (m_data->error) return false;

    int res = 0;
    if (!m_data->initialized) {
        if (!prepare()) return false;

        int retries = 0;
    first_frame:
        if (retries >= 8) {
            // fatal error
            m_data->error = true;
            return false;
        }
        res = mad_frame_decode(&m_data->frame, &m_data->stream);
        if (res != 0) {
            if (m_data->stream.error == MAD_ERROR_BUFLEN) {
                retries++;
                if (!moreData(true)) return false;
            }
            else
            if (m_data->stream.error == MAD_ERROR_LOSTSYNC)
            {}
            else
            if (MAD_RECOVERABLE(m_data->stream.error)) {
                #ifdef MPEG_DEBUG
                cerr << "Invalid frame: " << mad_stream_errorstr(&m_data->stream) << "\n";
                #endif
                m_data->metaframe_filter(true);
            }
            goto first_frame;
        }

        m_data->config.sample_rate = m_data->frame.header.samplerate;
        m_data->config.sample_width = 16;
        setChannelConfiguration(m_data->config, m_data->frame.header.mode);

        m_data->bitrate = m_data->frame.header.bitrate;
        m_data->layer = m_data->frame.header.layer;

        m_data->metaframe_filter(false);
    }
    else {
        if (m_data->stream.buffer == 0 ||
            m_data->stream.error==MAD_ERROR_BUFLEN ||
            m_data->stream.error==MAD_ERROR_LOSTSYNC) if (!moreData()) return false;

        int retries = 0;
        bool sync = true;
    retry:
        // give up after 16 new pages
        if (retries >= 16) {
            // fatal error
            m_data->error = true;
            return false;
        }
        res = mad_frame_decode(&m_data->frame, &m_data->stream);

        if (res != 0) {
            if (m_data->stream.error == MAD_ERROR_BUFLEN) {
                retries++;
                if (moreData(!sync)) return false;
                goto retry;
            }
            else
            if (m_data->stream.error == MAD_ERROR_LOSTSYNC) {
                #ifdef MPEG_DEBUG
                cerr << "No sync\n";
                #endif
                sync = false;
                goto retry;
            }
            else
            if (MAD_RECOVERABLE(m_data->stream.error)) {  // invalid frame
                #ifdef MPEG_DEBUG
                cerr << "Invalid frame: " << mad_stream_errorstr(&m_data->stream) << "\n";
                #endif
                m_data->metaframe_filter(true);
                goto retry;
            }
            else {
                // Unrecoverable error
                m_data->error = true;
                return false;
            }
        }
        // fast check for metaframes
        if (m_data->metaframe_filter(false))
            goto retry;
    }

    if (m_data->bitrate && !m_data->xing_vbr)
        if (m_data->bitrate != (int)m_data->frame.header.bitrate)
            m_data->unknown_vbr = true;

    mad_synth_frame(&m_data->synth, &m_data->frame);
    mad_pcm *pcm = &m_data->synth.pcm;

    // Has the format changed? (misread initial frame)
    m_data->config.sample_rate = pcm->samplerate;
    setChannelConfiguration(m_data->config, m_data->frame.header.mode);

    int channels = pcm->channels;
    long length = pcm->length;

    frame->reserveSpace(&m_data->config, length);

    // Scale into frame (could be reduced to copy if we used MADs internal format)
    int16_t** data = (int16_t**)frame->data;
    for(int j=0; j<channels; j++)
        for(int i=0; i<length; i++)
            data[j][i] = scale<16>(pcm->samples[j][i]);

    m_data->position += length;
    frame->pos = postotime(m_data->position, m_data->config.sample_rate);

    return true;
}

static inline long time_madtoakode(mad_timer_t time) {
    return mad_timer_count(time, MAD_UNITS_MILLISECONDS);
}

static float mpeg_length(MPEGDecoder::private_data *m_data) {
    if (m_data->layer == 1)
        return (m_data->xing.frames*384.0)/m_data->config.sample_rate;
    else
        return (m_data->xing.frames*1152.0)/m_data->config.sample_rate;
}

long MPEGDecoder::length() {
    float len;
    if (!m_data->initialized) return -1;
    else {
        if (m_data->xing_vbr || m_data->unknown_vbr) {
            if (m_data->xing_vbr && m_data->xing.frames > 0) {
                len = mpeg_length(m_data)*1000.0;
            }
            else { // VBR shot in the dark (based on average bitrate)
                if (m_data->length > 0) {
                     float bytepos = m_data->src->position()/(float)m_data->length;
                    len = position()/bytepos;
                } else
                    len = 0;
            }
        }
        else {
            if (m_data->length > 0)
                len = ((float)m_data->length*8000.0)/((float)m_data->bitrate);
            else
                len = 0;
        }
    }
    return (long)(len);
}

long MPEGDecoder::position() {
    if (!m_data->initialized) return -1;
    if (m_data->frame.header.samplerate > 0 && m_data->position >= 0)
        return postotime(m_data->position, m_data->config.sample_rate);
    else
        return -1;
}

bool MPEGDecoder::eof() {
    return m_data->eof;
}

bool MPEGDecoder::error() {
    return m_data->error;
}

bool MPEGDecoder::seek(long pos) {
    if (!m_data->initialized) return false;
    bool res = false;
    // VBR with toc:
    if (m_data->xing_vbr && m_data->xing.has_toc && m_data->xing.frames > 0) {
        int procent = (int)((float)pos/(mpeg_length(m_data)*10));
        float place = ((float)m_data->xing.toc[procent])/256.0;
        long bpos = (long)(place * m_data->length);
        #ifdef MPEG_DEBUG
        cerr << "TOC seeking " << procent << "/100 -> " << (long)(place*256.0) << "/256\n" ;
        #endif
        res = m_data->src->seek(bpos);
        if (res)
            m_data->position = (long)(((float)procent/100.0)*mpeg_length(m_data)*m_data->config.sample_rate);
    }
    else { // CBR seeking (and VBR without TOC fallback)
        #ifdef MPEG_DEBUG
        cerr << "CBR seeking\n";
        #endif
        long bpos = (long)((((float)pos)*m_data->bitrate)/8000.0);
        bpos += m_data->id3v2size;
        res = m_data->src->seek(bpos);
        if (res)
            m_data->position = (long)(((float)pos*(float)m_data->config.sample_rate)/1000.0);
    }
    if (res) {
        moreData(true);
        sync();
    }

    return res;
}

const AudioConfiguration* MPEGDecoder::audioConfiguration() {
    if (!m_data->initialized) return 0;
    return &m_data->config;
}

} // namespace
