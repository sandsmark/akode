/*  aKode: ALSA-Sink

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

#ifndef _AKODE_ALSA_SINK_H
#define _AKODE_ALSA_SINK_H

#include "sink.h"

#include "akode_export.h"

namespace aKode {

class AudioConfiguration;
class AudioFrame;

class ALSASink : public Sink {
public:
    ALSASink();
    ~ALSASink();
    bool open();
    void close();
    int setAudioConfiguration(const AudioConfiguration *config);
    const AudioConfiguration* audioConfiguration() const;
    // Writes blocking
    bool writeFrame(AudioFrame *frame);

    struct private_data;
private:
    template<class T> bool _writeFrame(AudioFrame *frame);
    private_data *m_data;
};

class ALSASinkPlugin : public SinkPlugin {
public:
    virtual ALSASink* openSink() {
        return new ALSASink();
    }
};

extern "C" AKODE_EXPORT ALSASinkPlugin alsa_sink;

} // namespace

#endif
