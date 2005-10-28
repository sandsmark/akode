/*  aKode: OSS-Sink

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

#ifndef _AKODE_OSS_SINK_H
#define _AKODE_OSS_SINK_H

#include "sink.h"

#include "akode_export.h"

namespace aKode {

class AudioConfiguration;
class AudioFrame;

class OSSSink : public Sink {
public:
    OSSSink();
    ~OSSSink();
    bool open();
    bool openDevice(const char*);
    void close();
    int setAudioConfiguration(const AudioConfiguration *config);
    const AudioConfiguration* audioConfiguration() const;
    // Writes blocking
    bool writeFrame(AudioFrame *frame);

//     void pause();
//     void resume();

    struct private_data;
private:
    template<class T> void _writeFrame(AudioFrame *frame);
    private_data *d;
};

class OSSSinkPlugin : public SinkPlugin {
public:
    virtual OSSSink* openSink() {
        return new OSSSink();
    }
};

extern "C" AKODE_EXPORT OSSSinkPlugin oss_sink;

} // namespace

#endif
