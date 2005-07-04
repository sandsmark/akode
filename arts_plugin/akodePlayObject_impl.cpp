/*  akodePlayObject

    Copyright (C) 2003-2005 Allan Sandfeld Jensen <kde@carewolf.com>

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

//#define AKODEARTS_SINGLETHREADED
//#define AKODEARTS_SRCRESAMPLING
#define AKODEARTS_FRAMEBUFFER 32
// The instream-buffer must be smaller than 64kbytes (1<<16)
#define AKODEARTS_INSTREAMBUFFER (1<<14)

#include <math.h>

#include <soundserver.h>
#include <audiosubsys.h>
#include <debug.h>

#include "config.h"
#include "wav_decoder.h"

#include "audioframe.h"
#include "localfile.h"
#include "mmapfile.h"
#include "framedecoder.h"
#include "streamdecoder.h"
#include "arts_inputstream.h"

#include "resampler.h"
#include "fast_resampler.h"

#ifndef AKODEARTS_SINGLETHREADED
 #include "buffered_decoder.h"
#endif

#ifdef AKODEARTS_SRCRESAMPLING
#define AKODEARTS_RESAMPLER "src"
#else
#define AKODEARTS_RESAMPLER "fast"
#endif


#include "akodePlayObject_impl.h"

using namespace Arts;
using namespace aKode;

akodePlayObject_impl::akodePlayObject_impl(const string &plugin)
  : source(0)
  , frameDecoder(0)
  , decoder(0)
  , bufferedDecoder(0)
  , resampler(0)
  , buffer(0)
  , inBuffer(0)
  , buf_pos(0)
  , mState(posIdle)
  , mSpeed(1.0)
  , m_packetQueue(0)
  , m_bytebuffer(0)
  , m_fading(false)
  , decoderPlugin(plugin)
  , resamplerPlugin(AKODEARTS_RESAMPLER)
{
    m_packetQueue = new queue<DataPacket<mcopbyte>*>;
    if(!resamplerPlugin.isLoaded())
        resamplerPlugin.load("fast");
}

akodePlayObject_impl::~akodePlayObject_impl()
{
    unload();
}

bool akodePlayObject_impl::loadPlugin(const string &plugin)
{
    return decoderPlugin.load(plugin);
}

bool akodePlayObject_impl::streamMedia(Arts::InputStream inputstream)
{
    arts_debug("akode: opening input-stream");
    m_bytebuffer = new aKode::ByteBuffer(AKODEARTS_INSTREAMBUFFER);
    instream = inputstream;

    Arts::StreamPlayObject self = Arts::StreamPlayObject::_from_base(_copy());
    connect(instream, "outdata", self, "indata");

    source = new Arts_InputStream(instream, m_bytebuffer);
    return loadSource();
}

bool akodePlayObject_impl::loadMedia(const string &filename)
{
    arts_debug("akode: opening %s", filename.c_str());
    source = new MMapFile(filename.c_str());
    if (!source->openRO()) {
        delete source;
        source = new LocalFile(filename.c_str());
        if (!source->openRO()) {
            delete source;
            source = 0;
            return false;
        }
    }
    source->close();
    return loadSource();
}

bool akodePlayObject_impl::loadSource()
{
    if (!decoderPlugin.isLoaded()) {
        return false;
    }
    frameDecoder = decoderPlugin.openFrameDecoder(source);

    if (!frameDecoder) {
        delete source;
        source = 0;
        arts_warning("akode: Could not open frame-decoder");
        return false;
    }

#ifndef AKODEARTS_SINGLETHREADED
    bufferedDecoder = new BufferedDecoder();
    bufferedDecoder->setBufferSize(AKODEARTS_FRAMEBUFFER);
    bufferedDecoder->openDecoder(frameDecoder);
    decoder = bufferedDecoder;
#else
    decoder = frameDecoder;
#endif

    return true;
}

string akodePlayObject_impl::description()
{
    return "akodePlayObject";
}

Arts::InputStream akodePlayObject_impl::inputStream() {
    return instream;
}

string akodePlayObject_impl::mediaName()
{
    if (source)
        return source->filename;
    else
        return string();
}

poCapabilities akodePlayObject_impl::capabilities()
{
    return (poCapabilities)(capPause | capSeek);
}

poState akodePlayObject_impl::state()
{
    return mState;
}

void akodePlayObject_impl::play()
{
    arts_debug("akode: play");

    if (!decoder) {
        arts_warning("akode: No media loaded");
	return;
    }

    if (mState == posIdle) {
        mState = posPlaying;

        if (!inBuffer) inBuffer = new AudioFrame;
	if (!buffer) buffer = inBuffer;


	buf_pos = 0;
    } else
        mState = posPlaying;
}

void akodePlayObject_impl::pause()
{
    arts_debug("akode: pause");
    mState = posPaused;
}

void akodePlayObject_impl::halt()
{
    arts_debug("akode: halt");
    if (mState == posIdle) return;
    mState = posIdle;
    unload();
}

void akodePlayObject_impl::unload()
{
    arts_debug("akode: unload");
    if (m_bytebuffer) m_bytebuffer->release();
#ifndef AKODEARTS_SINGLETHREADED
    if (bufferedDecoder) {
        bufferedDecoder->stop();
        bufferedDecoder->closeDecoder();
        delete bufferedDecoder;
        bufferedDecoder = 0;
    }
#else
    delete frameDecoder
    frameDecoder = 0;
#endif
    decoder = 0;
    if (buffer != inBuffer)
        delete inBuffer;
    delete buffer;
    inBuffer = buffer = 0;
    buf_pos = 0;

    delete resampler;
    resampler = 0;
    delete source;
    source = 0;
#ifndef AKODEARTS_SINGLETHREADED
    delete m_bytebuffer;
    m_bytebuffer = 0;
#endif
}

poTime akodePlayObject_impl::currentTime()
{
    poTime time;

    long pos;
    if (decoder) {
        pos = decoder->position();            // decoder time
        if (pos < 0 ) pos = 0;
        else
        if (samplingRate > 0 && buffer)
        {
            float lpos = (float)(buf_pos-buffer->length) / (float)samplingRate;  // local time
            pos += (long)(lpos*1000.0);
        }
    }
    else
        pos = 0;

    time.seconds = pos / 1000 ;
    time.ms = pos % 1000;

    return time;
}

poTime akodePlayObject_impl::overallTime()
{
    poTime time;

    long len;
    if (decoder) {
        len = decoder->length();
        if (len < 0 ) len = 0;
    }
    else
        len = 0;

    time.seconds = len / 1000;
    time.ms = len % 1000;

    return time;
}

void akodePlayObject_impl::seek(const poTime &time)
{
    arts_debug("akode: seek");
    if (!decoder) {
        arts_warning("akode: No media loaded");
	return;
    }
    long akode_pos = time.seconds*1000+time.ms;

    if (decoder->seek(akode_pos) && buffer) {
        buffer->length = 0;  // force re-read
        buf_pos = 0;
    }
}

bool akodePlayObject_impl::readFrame()
{
    arts_debug("akode: readFrame");

    if (!buffer || !decoder) return false;
    if (m_bytebuffer) processQueue();
    if(!decoder->readFrame(inBuffer)) {
        if (decoder->eof()) {
            arts_debug("akode: eof");
            halt();
        } else
        if (decoder->error()) {
            arts_debug("akode: error");
            halt();
        } else
            buffer->length=0;
        return false;
    }

    // Invalid frame from broken plugin
    if (inBuffer->sample_rate == 0) return false;

    if (samplingRate != inBuffer->sample_rate || mSpeed != 1.0) {
        //arts_debug("akode: resampling to %d/%d", inBuffer->sample_rate, samplingRate);
        if ( !buffer || buffer==inBuffer ) buffer = new AudioFrame;
        if ( !resampler)
            resampler = resamplerPlugin.openResampler();

        resampler->setSampleRate(samplingRate);
        resampler->setSpeed(mSpeed);

        resampler->doFrame(inBuffer, buffer);
    } else {
        if ( buffer !=inBuffer) delete buffer;
        buffer = inBuffer;
    }

    buf_pos = 0;
    return true;
}

bool akodePlayObject_impl::eof()
{
    if (!decoder || !buffer) return true;
    else
        return buf_pos>=buffer->length && decoder->eof();
}

// GCC's lack of good template support means this is easyist done using DEFINE
#define SEND_BLOCK(T)  \
    T* data = (T*)buffer->data[0]; \
    j = i; bp = buf_pos; \
    while (bp < buffer->length && j<count) { \
    	left[j] = data[bp]*scale; \
        j++; bp++; \
    } \
    if (buffer->channels > 1) \
    	data = (T*)buffer->data[1]; \
    j = i; bp = buf_pos; \
    while (bp < buffer->length && j<count) { \
    	right[j] = data[bp]*scale; \
    	j++; bp++; \
    }


void akodePlayObject_impl::calculateBlock(unsigned long cnt)
{
    long count = (long)cnt;
//    arts_debug("akode: calculateBlock");
    long i = 0, j, bp;

    if (!decoder) {
        arts_warning("akode: No media loaded");
	goto fill_out;
    }

    if (!buffer)
        // Not playing yet
        goto fill_out;

    while ((mState == posPlaying || m_fading) && i<count) {
	if (buf_pos >= buffer->length) {
            buf_pos = 0;
	    if (!readFrame()) {
                break;
	    }
	}
        if (buffer->channels > 2 || buffer->sample_width > 24 || buffer->sample_width == 0) {
            arts_warning("akode: Incompatible media");
	    halt();
	    break;
	}

        signed char width = buffer->sample_width;
        float scale = (float)(1<<(width-1));
	if (width >= 0) {
            scale = 1/scale;
            if (width <= 8) {
	       SEND_BLOCK(int8_t);
	       i=j;
               buf_pos=bp;
            }
	    else if (width <= 16) {
	       SEND_BLOCK(int16_t);
	       i=j;
               buf_pos=bp;
	   }
           else {
	       SEND_BLOCK(int32_t);
	       i=j;
               buf_pos=bp;
	   }
        }
        else {
            scale = 1.0;
            SEND_BLOCK(float);
            i=j;
            buf_pos=bp;
        }
    }

    // fill-out the rest with silence
fill_out:
    for (; i < count; i++) {
    	left[i] = right[i] = 0;
    }

}

void akodePlayObject_impl::streamInit()
{
    arts_debug("akode: streamInit");
}

void akodePlayObject_impl::streamStart()
{
    arts_debug("akode: streamStart");
#ifndef AKODEARTS_SINGLETHREADED
    bufferedDecoder->start();
#endif
}

void akodePlayObject_impl::streamEnd()
{
    arts_debug("akode: streamEnd");
    mState = posIdle;
    if (decoder) unload();
}

void akodePlayObject_impl::speed(float newValue)
{
    mSpeed = newValue;
}

float akodePlayObject_impl::speed()
{
    return mSpeed;
}

void akodePlayObject_impl::process_indata(DataPacket<mcopbyte> *inpacket) {
    arts_debug("akode: process_indata");
    m_packetQueue->push(inpacket);
    if (!m_bytebuffer) return;
    processQueue();
}

void akodePlayObject_impl::processQueue()
{
    while (!m_packetQueue->empty()) {
        long freespace = m_bytebuffer->space();

        DataPacket<mcopbyte> *inpacket = m_packetQueue->front();
        if (!inpacket) return;

        if (freespace >= inpacket->size) {
            if (m_bytebuffer->write((char*)inpacket->contents, inpacket->size, false)) {
                m_packetQueue->pop();
                inpacket->processed();
            }
        } else
            return;
    }
    if (instream.eof()) m_bytebuffer->close();
}


REGISTER_IMPLEMENTATION(akodePlayObject_impl);
