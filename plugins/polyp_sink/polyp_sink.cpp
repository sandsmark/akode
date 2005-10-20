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

#include "config.h"

#ifdef HAVE_STDINT_H
#include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#endif
#include <polyp/polyplib-simple.h>

#include "audioframe.h"
#include "audiobuffer.h"
#include "polyp_sink.h"

#include <iostream>

namespace aKode {

extern "C" { PolypSinkPlugin polyp_sink; };

struct PolypSink::private_data
{
    private_data() : server(0), error(false) {};

    pa_simple *server;
    pa_sample_spec sample_spec;

    bool error;

    AudioConfiguration config;
};

PolypSink::PolypSink()
{
    m_data = new private_data;
    m_data->sample_spec.rate = 44100;
    m_data->sample_spec.channels = 2;
    m_data->sample_spec.format = PA_SAMPLE_S16NE;
}

bool PolypSink::open() {
    int error = 0;
    m_data->server = pa_simple_new(0, "akode-client", PA_STREAM_PLAYBACK, 0, "", &m_data->sample_spec, 0, 255, &error );
    if (!m_data->server || error != 0) {
        m_data->error = true;
        close();
        std::cout << "Cannot open client\n";
        return false;
    }


    return true;
}

void PolypSink::close() {
    if (m_data->server) {
        pa_simple_free(m_data->server);
        m_data->server = 0;
    }
}

PolypSink::~PolypSink()
{
    close();
    delete m_data;
}

int PolypSink::setAudioConfiguration(const AudioConfiguration* config)
{
    if (m_data->error) return -1;

    int res = 0;
    if (*config == m_data->config) return 0;
    m_data->config = *config;

    if (config->channel_config != MonoStereo ) return -1;
    m_data->sample_spec.channels = config->channels;

    if (config->sample_width != 16) {
        res = 1;
        m_data->config.sample_width = 16;
    }

    m_data->sample_spec.rate = config->sample_rate;

    // create new connection
    close();
    open();

    return res;
}

const AudioConfiguration* PolypSink::audioConfiguration() const
{
    return &m_data->config;
}

bool PolypSink::writeFrame(AudioFrame* frame)
{
    if ( m_data->error ) return false;

    if ( frame->channels != m_data->config.channels || frame->sample_rate != m_data->config.sample_rate)
    {
        if (setAudioConfiguration(frame)!=0)
            return false;
    }

    int channels = m_data->config.channels;
    int length = frame->length;

    int16_t *buffer = new int16_t[length*channels];
    int16_t** data = (int16_t**)frame->data;
    for(int i = 0; i<length; i++)
        for(int j=0; j<channels; j++)
            buffer[i*channels+j] = data[j][i];

    int error = 0;
    pa_simple_write(m_data->server, buffer, channels*length*2, &error);
    delete[] buffer;

    return (error == 0);
}

} // namespace
