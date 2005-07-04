/*  akodeSpeexStreamPlayObject

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

#include "akodeSpeexStreamPlayObject_impl.h"
#include "framedecoder.h"
#include "buffered_decoder.h"

akodeSpeexStreamPlayObject_impl::akodeSpeexStreamPlayObject_impl()
  : akodeSpeexStreamPlayObject_skel(), akodePlayObject_impl("xiph")
{
    speex_plugin = (aKode::DecoderPlugin*)decoderPlugin.loadPlugin("speex_decoder");
}


bool akodeSpeexStreamPlayObject_impl::loadSource()
{
    frameDecoder = speex_plugin->openFrameDecoder(source);
#ifndef SINGLETHREADED_AKODE_PLUGIN
    bufferedDecoder = new aKode::BufferedDecoder();
    bufferedDecoder->openDecoder(frameDecoder);
    decoder = bufferedDecoder;
#else
    decoder = frameDecoder;
#endif
    return true;
}

REGISTER_IMPLEMENTATION(akodeSpeexStreamPlayObject_impl);



