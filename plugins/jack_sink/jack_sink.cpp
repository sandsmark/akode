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

#include "config.h"

#ifdef HAVE_STDINT_H
#include <stdint.h>
#elif HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include <jack/jack.h>

#include <audioframe.h>
#include <audiobuffer.h>
#include "jack_sink.h"

#include <iostream>

namespace aKode {

extern "C" { JACKSinkPlugin jack_sink; }

struct JACKSink::private_data
{
    private_data() : left_port(0), right_port(0), client(0), error(false), sample_rate(0), buffer(8),pos(0) {};
//    jack_port_t *input_port;
    jack_port_t *left_port;
    jack_port_t *right_port;

    jack_client_t *client;

    bool error;
    unsigned int sample_rate;

    AudioConfiguration config;
    AudioBuffer buffer;
    AudioFrame current;
    int pos;
};

static int process (jack_nframes_t nframes, void *arg)
{
    JACKSink::private_data *m_data = (JACKSink::private_data*)arg;

    jack_default_audio_sample_t *out1=0, *out2=0;

    if (m_data->left_port)
        out1 = (jack_default_audio_sample_t *) jack_port_get_buffer (m_data->left_port, nframes);
    if (m_data->right_port)
        out2 = (jack_default_audio_sample_t *) jack_port_get_buffer (m_data->right_port, nframes);

    if (!out2 && !out1) return 0;

    unsigned int n = 0;
    float** buffer = (float**)m_data->current.data;
    while(n < nframes) {
        if (m_data->pos >= m_data->current.length) {
            if (!m_data->buffer.get(&m_data->current, false)) break;
            m_data->pos = 0;
            buffer = (float**)m_data->current.data;
        }

        if (out1) out1[n] = buffer[0][m_data->pos];
        if (out2) out2[n] = buffer[1][m_data->pos];

        n++;
        m_data->pos++;
    }

    return n;
}

static void shutdown (void *arg)
{
    JACKSink::private_data *m_data = (JACKSink::private_data*)arg;

    // Jack has shut down, so the sink is in fatal error.
    m_data->error = true;
    m_data->buffer.release();
}

JACKSink::JACKSink()
{
    m_data = new private_data;
}

bool JACKSink::open() {
    m_data->client = jack_client_new("akode_client");
    if (!m_data->client) {
        m_data->error = true;
        std::cout << "Cannot open client\n";
        return false;
    }
    jack_set_process_callback(m_data->client, process, (void*)m_data);
    jack_on_shutdown(m_data->client, shutdown, (void*)m_data);

    m_data->sample_rate = jack_get_sample_rate(m_data->client);
//    jack_set_sample_rate_callback(m_data->client, sample_rate, (void*)m_data);

    if (jack_activate(m_data->client)) {
        m_data->error = true;
        std::cout << "Activate failed\n";
        return false;
    }
    const char** names = jack_get_ports (m_data->client, NULL, NULL, JackPortIsPhysical|JackPortIsInput);
    while (*names) {
        std::cout << *names << std::endl;
        names++;
    }

    return true;
}

JACKSink::~JACKSink()
{
    if (m_data->left_port)
        jack_port_unregister(m_data->client, m_data->left_port);
    if (m_data->right_port)
        jack_port_unregister(m_data->client, m_data->right_port);
    if (m_data->client)
        jack_deactivate(m_data->client);
    delete m_data;
}

int JACKSink::setAudioConfiguration(const AudioConfiguration* config)
{
    if (m_data->error) return -1;

    int res = 0;
    m_data->config = *config;

    if (config->channel_config != MonoStereo ) return -1;
    m_data->left_port = jack_port_register (m_data->client, "left", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if (jack_connect (m_data->client, jack_port_name (m_data->left_port), "alsa_pcm:playback_1")) {
        m_data->error = true;
        return -1;
    }
    if (config->channels > 1) {
        m_data->right_port = jack_port_register (m_data->client, "right", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
        if (jack_connect (m_data->client, jack_port_name (m_data->right_port), "alsa_pcm:playback_2")) {
            m_data->config.channels = 1;
            res = 1;
        }
    }

    if (config->sample_width != -32) {
        res = 1;
        m_data->config.sample_width = -32;
    }

    if (config->sample_rate != m_data->sample_rate) {
        res = 1;
        m_data->config.sample_rate = m_data->sample_rate;
    }

    return res;
}

const AudioConfiguration* JACKSink::audioConfiguration() const
{
    return &m_data->config;
}

template<typename S>
void JACKSink::convertFrame(AudioFrame *in, AudioFrame *out)
{
    float scale = (float)(1<<(in->sample_width-1));
    scale = 1.0/scale;
//    float scale = 1;

    out->reserveSpace(&m_data->config, in->length);

    int channels = in->channels;
    S** indata = (S**)in->data;
    float** outdata = (float**)out->data;
    for(int j=0; j<in->length; j++) {
        for(int i=0; i<channels; i++) {
            outdata[i][j] = scale*indata[i][j];
        }
    }
}

bool JACKSink::writeFrame(AudioFrame* frame)
{
    if ( m_data->error ) return false;

    if ( frame->channels != m_data->config.channels )
    {
        if (setAudioConfiguration(frame)!=0)
            return false;
    }

    if (frame->sample_width>0) {
        AudioFrame out;
        if (frame->sample_width<=8)
            convertFrame<int8_t>(frame, &out);
        else
        if (frame->sample_width<=16)
            convertFrame<int16_t>(frame, &out);
        else
        if (frame->sample_width<=32)
            convertFrame<int32_t>(frame, &out);
        return m_data->buffer.put(&out, true);
    } else
        return m_data->buffer.put(frame, true);

}

} // namespace
