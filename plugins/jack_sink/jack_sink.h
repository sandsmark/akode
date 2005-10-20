/*  aKode: JACK-Sink

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

#ifndef _AKODE_JACK_SINK_H
#define _AKODE_JACK_SINK_H

#include "sink.h"

#include "akode_export.h"

namespace aKode {

class AudioConfiguration;
class AudioFrame;

class JACKSink : public Sink {
public:
    JACKSink();
    ~JACKSink();
    bool open();
    int setAudioConfiguration(const AudioConfiguration *config);
    const AudioConfiguration* audioConfiguration() const;
    // Writes blocking
    bool writeFrame(AudioFrame *frame);

    struct private_data;
private:
    template<class T> void convertFrame(AudioFrame *in, AudioFrame *out);
    private_data *m_data;
};

class JACKSinkPlugin : public SinkPlugin {
public:
    virtual JACKSink* openSink() {
        return new JACKSink();
    }
};

extern "C" AKODE_EXPORT JACKSinkPlugin jack_sink;

} // namespace

#endif
