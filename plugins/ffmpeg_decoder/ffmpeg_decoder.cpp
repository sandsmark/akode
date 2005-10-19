/*  aKode: FFMPEG Decoder

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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "akodelib.h"
// #ifdef HAVE_FFMPEG

#include "file.h"
#include "audioframe.h"
#include "decoder.h"

#include <assert.h>
#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>
#include <ffmpeg/avio.h>

#include "ffmpeg_decoder.h"
#include <iostream>

// FFMPEG callbacks
extern "C" {
    static int akode_read(void* opaque, unsigned char *buf, int size)
    {
        aKode::File *file = (aKode::File*)opaque;
        return file->read((char*)buf, size);
    }
    static int akode_write(void* opaque, unsigned char *buf, int size)
    {
        aKode::File *file = (aKode::File*)opaque;
        return file->write((char*)buf, size);
    }
    static offset_t akode_seek(void* opaque, offset_t pos, int whence)
    {
        aKode::File *file = (aKode::File*)opaque;
        return file->seek(pos, whence);
    }
}

namespace aKode {


bool FFMPEGDecoderPlugin::canDecode(File* /*src*/) {
    // ### FIXME
    return true;
}
/*
void FFMPEGDecoderPlugin::initializeFFMPEG() {
    av_register_all();
}*/

extern "C" { FFMPEGDecoderPlugin ffmpeg_decoder; }

#define FILE_BUFFER_SIZE 8192

struct FFMPEGDecoder::private_data
{
    private_data() : packetSize(0), position(0),
                     eof(false), error(false), initialized(false), retries(0) {};

    AVFormatContext* ic;
    AVCodec* codec;
    AVInputFormat *fmt;
    ByteIOContext stream;

    AVPacket packet;
    uint8_t* packetData;
    int packetSize;

    File *src;
    AudioConfiguration config;

    long position;

    bool eof, error;
    bool initialized;
    int retries;

    unsigned char file_buffer[FILE_BUFFER_SIZE];
    char buffer[AVCODEC_MAX_AUDIO_FRAME_SIZE];
    int buffer_size;
};

FFMPEGDecoder::FFMPEGDecoder(File *src) {
    d = new private_data;
    av_register_all();

    d->src = src;
}

FFMPEGDecoder::~FFMPEGDecoder() {
    closeFile();
    delete d;
}


static bool setAudioConfiguration(AudioConfiguration *config, AVCodecContext *codec_context)
{
    config->sample_rate = codec_context->sample_rate;
    config->channels = codec_context->channels;
    // I do not know FFMPEGs surround channel ordering
    if (config->channels > 2) return false;
    config->channel_config = MonoStereo;
    switch(codec_context->sample_fmt) {
        case SAMPLE_FMT_S16:
            config->sample_width = 16;
            break;
        case SAMPLE_FMT_S32:
            config->sample_width = 32;
            break;
        case SAMPLE_FMT_FLT:
            config->sample_width = -32;
            break;
        case SAMPLE_FMT_DBL:
            config->sample_width = -64;
            break;
        default:
            return false;;
     }
     return true;
}


bool FFMPEGDecoder::openFile() {
    d->src->openRO();
    d->src->fadvise();

    // The following duplicates what av_file_open would normally do

    // url_fdopen
    init_put_byte(&d->stream, d->file_buffer, FILE_BUFFER_SIZE, 0, d->src, akode_read, akode_write, akode_seek);
    d->stream.is_streamed = !d->src->seekable();
    d->stream.max_packet_size = FILE_BUFFER_SIZE;

    {
        // 2048 is PROBE_BUF_SIZE from libavformat/utils.c
        AVProbeData pd;
        uint8_t buf[2048];
        pd.filename = d->src->filename;
        pd.buf = buf;
        pd.buf_size = 0;
        pd.buf_size = get_buffer(&d->stream, buf, 2048);
        d->fmt = av_probe_input_format(&pd, 1);
        // Seek back to 0
        // copied from url_fseek
        long offset1 = 0 - (d->stream.pos - (d->stream.buf_end - d->stream.buffer));
        if (offset1 >= 0 && offset1 <= (d->stream.buf_end - d->stream.buffer)) {
            /* can do the seek inside the buffer */
            d->stream.buf_ptr = d->stream.buffer + offset1;
        } else
            if (!d->src->seek(0)) {
                d->src->close();
                return false;
            } else {
                d->stream.pos = 0;
                d->stream.buf_ptr = d->file_buffer;
                d->stream.buf_end = d->file_buffer;
            }
        }
    }
    if (!d->fmt) {
        std::cerr << "akode: FFMPEG: Format not found\n";
        closeFile();
        return false;
    }

    if (av_open_input_stream(&d->ic, &d->stream, d->src->filename, d->fmt, 0) != 0)
    {
        closeFile();
        return false;
    }

    av_find_stream_info( d->ic );

    // sanety check
    if (d->ic->nb_streams != 1 || d->ic->streams[0]->codec->codec_type != CODEC_TYPE_AUDIO)
    {
        // for now only accept single stream audio tracks
        closeFile();
        return false;
    }

    // Set config
    if (!setAudioConfiguration(&d->config, d->ic->streams[0]->codec))
    {
        closeFile();
        return false;
    }

    d->codec = avcodec_find_decoder(d->ic->streams[0]->codec->codec_id);
    if (!d->codec) {
        std::cerr << "akode: FFMPEG: Codec not found\n";
        closeFile();
        return false;
    }
    avcodec_open( d->ic->streams[0]->codec, d->codec );

    double ffpos = (double)d->ic->streams[0]->start_time / (double)AV_TIME_BASE;
    d->position = (long)(ffpos * d->config.sample_rate);

    return true;
}

void FFMPEGDecoder::closeFile() {
    if( d->codec ) {
        avcodec_close( d->ic->streams[0]->codec );
        d->codec = 0;
    }
    if( d->ic ) {
        // make sure av_close_input_file doesn't actually close the file
        d->ic->iformat->flags = d->ic->iformat->flags | AVFMT_NOFILE;
        av_close_input_file( d->ic );
        d->ic = 0;
    }
    if (d->src) {
        d->src->close();
    }
}

bool FFMPEGDecoder::readPacket() {
    av_init_packet(&d->packet);
    if ( av_read_frame(d->ic, &d->packet) != 0 ) {
        av_free_packet( &d->packet );
        d->packetSize = 0;
        d->packetData = 0;
        return false;
    } else {
        d->packetSize = d->packet.size;
        d->packetData = d->packet.data;
        return true;
    }
}

template<typename T>
static long demux(FFMPEGDecoder::private_data* d, AudioFrame* frame) {
    int channels = d->config.channels;
    long length = d->buffer_size/(channels*sizeof(T));
    frame->reserveSpace(&d->config, length);

    // Demux into frame
    T* buffer = (T*)d->buffer;
    T** data = (T**)frame->data;
    for(int i=0; i<length; i++)
        for(int j=0; j<channels; j++)
            data[j][i] = buffer[i*channels+j];
    return length;
}

bool FFMPEGDecoder::readFrame(AudioFrame* frame)
{
    if (!d->initialized) {
        if (!openFile()) {
            d->error = true;
            return false;
        }
        d->initialized = true;
    }

    if( d->packetSize <= 0 )
        if (!readPacket()) {
            std::cerr << "akode: FFMPEG: EOF guessed\n";
            d->eof = true;
            return false;
        }
retry:
    int len = avcodec_decode_audio( d->ic->streams[0]->codec,
                                    (short*)d->buffer, &d->buffer_size,
                                    d->packetData, d->packetSize );

    if (len <= 0) {
        d->retries++;
        if (d->retries > 8) {
            std::cerr << "akode: FFMPEG: Decoding failure\n";
            d->error = true;
            return false;
        }
        goto retry;
    } else
        d->retries = 0;

    d->packetSize -= len;
    d->packetData += len;
    long length = 0;
    switch (d->config.sample_width) {
        case 16:
            length = demux<int16_t>(d,frame);
            break;
        case 32:
            length = demux<int32_t>(d,frame);
            break;
        case -32:
            length = demux<float>(d,frame);
            break;
        case -64:
            length = demux<double>(d,frame);
            break;
        default:
            assert(false);
    }
    if (length == 0) return readFrame(frame);
    std::cout << "Frame length: " << length << "\n";

    if( d->packetSize <= 0 )
      av_free_packet( &d->packet );

    frame->pos = (d->position*1000)/d->config.sample_rate;
    d->position += length;
    return true;
}

long FFMPEGDecoder::length() {
    if (!d->initialized) return -1;
    // ### Returns only the length of the first stream
    double ffmpeglen = (double)d->ic->streams[0]->duration / (double)AV_TIME_BASE;
    return (long)(ffmpeglen*1000.0);
}

long FFMPEGDecoder::position() {
    if (!d->initialized) return -1;
    // ### Replace with FFMPEG native call
    return (d->position*1000)/d->config.sample_rate;
}

bool FFMPEGDecoder::eof() {
    return d->eof;
}

bool FFMPEGDecoder::error() {
    return d->error;
}

bool FFMPEGDecoder::seekable() {
    return d->src->seekable();
}

bool FFMPEGDecoder::seek(long pos) {
    if (!d->initialized) return false;
    AVRational time_base = d->ic->streams[0]->time_base;
    std::cout<< "time base is " << time_base.num << "/" << time_base.den << "\n";
    long ffpos = 0;
    {
        int div = (pos / (time_base.num*1000)) * time_base.den;
        int rem = (pos % (time_base.num*1000)) * time_base.den;
        ffpos = div + rem / (time_base.num*1000);
    }
    std::cout<< "seeking to " << pos << "ms\n";
//     long ffpos = (long)((pos/1000.0)*AV_TIME_BASE);
    std::cout<< "seeking to " << ffpos << "pos\n";
    bool res = av_seek_frame(d->ic, 0, ffpos, 0);
    if (res < 0) return false;
    else {
        d->position = (pos * d->config.sample_rate)/1000;
        return true;
    }
}

const AudioConfiguration* FFMPEGDecoder::audioConfiguration() {
    if (!d->initialized) return 0;
    return &d->config;
}

} // namespace

// #endif // HAVE_FFMPEG
