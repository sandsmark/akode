/*  aKode: Polyp-Sink

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

#ifndef _AKODE_POLYP_SINK_H
#define _AKODE_POLYP_SINK_H

#include "sink.h"

#include "akode_export.h"

namespace aKode {

class AudioConfiguration;
class AudioFrame;

class PolypSink : public Sink {
public:
    PolypSink();
    ~PolypSink();
    bool open();
    void close();
    int setAudioConfiguration(const AudioConfiguration *config);
    const AudioConfiguration* audioConfiguration() const;
    // Writes blocking
    bool writeFrame(AudioFrame *frame);

    struct private_data;
private:
    private_data *m_data;
};

class PolypSinkPlugin : public SinkPlugin {
public:
    virtual PolypSink* openSink() {
        return new PolypSink();
    }
};

extern "C" AKODE_EXPORT PolypSinkPlugin polyp_sink;

} // namespace

#endif
