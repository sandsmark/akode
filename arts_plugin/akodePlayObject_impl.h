/*  akodePlayObject

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

#ifndef _AKODEPLAYOBJECT_IMPL_H
#define _AKODEPLAYOBJECT_IMPL_H

#include "resampler.h"
#include "audioframe.h"
#include "framedecoder.h"
#include "bytebuffer.h"
#include "resampler.h"

#include <stdsynthmodule.h>
#include <kmedia2.h>
#include <queue>
#include "akodearts.h"

using std::string;
using std::queue;

namespace aKode {
    class BufferedDecoder;
    class File;
}

class akodePlayObject_impl
    : virtual public akodePlayObject_skel
    , virtual public Arts::StdSynthModule
{
public:
	akodePlayObject_impl(const string &plugin = "wav");
	~akodePlayObject_impl();

	virtual bool loadMedia(const string &);
        virtual bool streamMedia(Arts::InputStream instream);
        virtual bool loadSource();
	string description();
	string mediaName();
        Arts::InputStream inputStream();
	Arts::poCapabilities capabilities();
	Arts::poState state();
	void play();
	void pause();
	void halt();
	void seek(const Arts::poTime &);
	Arts::poTime currentTime();
	Arts::poTime overallTime();

	void streamInit();
	void streamStart();
	void calculateBlock(unsigned long samples);
	void streamEnd();

        void speed(float newValue);
        float speed();

        void process_indata(Arts::DataPacket<Arts::mcopbyte>*);
        void processQueue();

protected:
        bool loadPlugin(const string &plugin);
	bool readFrame();
        bool eof();
	void unload();
        Arts::InputStream instream;
        aKode::File *source;
	aKode::FrameDecoder *frameDecoder, *decoder;
	aKode::BufferedDecoder *bufferedDecoder;
	aKode::Resampler *resampler;
	aKode::AudioFrame *buffer, *inBuffer;
	int buf_pos;
	Arts::poState mState;
        float mSpeed;
        queue<Arts::DataPacket<Arts::mcopbyte>*> *m_packetQueue;
        aKode::ByteBuffer *m_bytebuffer;
        bool m_fading;
        aKode::DecoderPluginHandler decoderPlugin;
        aKode::ResamplerPluginHandler resamplerPlugin;
};

#endif
