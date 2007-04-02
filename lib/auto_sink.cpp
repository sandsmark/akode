/*  aKode: Auto-Sink

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

#include "audioframe.h"
#include "auto_sink.h"

#include <cstdlib>
#include <iostream>

namespace aKode {

extern "C" { AutoSinkPlugin auto_sink; }

struct AutoSink::private_data
{
    private_data() : sink(0) {};
    SinkPluginHandler plugin_handler;
    Sink* sink;

    bool tryOpen(const string plugin) {
        if (plugin_handler.load(plugin)) {
            sink = plugin_handler.openSink();
            if (sink) {
                if (sink->open())
                    return true;
                delete sink;
                sink = 0;
            }
            plugin_handler.unload();
        } else
            std::cout << "auto_sink: Could not load " << plugin << std::endl;
        return false;
    }
};

AutoSink::AutoSink()
{
    m_data = new private_data;
}

AutoSink::~AutoSink()
{
    close();
    delete m_data->sink;
    delete m_data;
}

bool AutoSink::open()
{
    // Try Polypaudio
    if (getenv("POLYP_SERVER"))
        if (m_data->tryOpen("polyp")) return true;
    // Try Jack
    if (m_data->tryOpen("jack")) return true;
    // Try ALSA
    if (m_data->tryOpen("alsa")) return true;
    // Try OSS
    if (m_data->tryOpen("oss")) return true;
    // Try Sun Audio
    if (m_data->tryOpen("sun")) return true;
    // Fail
    return false;
}

void AutoSink::close()
{
    if (m_data->sink)
        m_data->sink->close();
}

int AutoSink::setAudioConfiguration(const AudioConfiguration* config)
{
    if (!m_data->sink)
        return -1;
    else
        return m_data->sink->setAudioConfiguration(config);
}

const AudioConfiguration* AutoSink::audioConfiguration() const
{
    if (!m_data->sink)
        return 0;
    else
        return m_data->sink->audioConfiguration();
}

bool AutoSink::writeFrame(AudioFrame* frame)
{
    if (!m_data->sink)
        return false;
    else
        return m_data->sink->writeFrame(frame);
}

} // namespace
