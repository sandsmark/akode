/*  aKode: Void-Sink

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <config.h>

#include "audioframe.h"
#include "void_sink.h"

//#include <iostream>

namespace aKode {

extern "C" { VoidSinkPlugin void_sink; }

struct VoidSink::private_data
{
    AudioConfiguration config;
};

VoidSink::VoidSink()
{
    m_data = new private_data;
}

VoidSink::~VoidSink()
{
    close();
    delete m_data;
}

bool VoidSink::open()
{
    return true;
}


void VoidSink::close()
{
}

int VoidSink::setAudioConfiguration(const AudioConfiguration* config)
{
    m_data->config = *config;

    return 0;
}

const AudioConfiguration* VoidSink::audioConfiguration() const
{
    return &m_data->config;
}

bool VoidSink::writeFrame(AudioFrame* /*frame*/)
{
    return true;
}

} // namespace
