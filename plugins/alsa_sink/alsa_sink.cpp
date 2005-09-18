/*  aKode: ALSA Sink

    Copyright (C) 2004-2005 Allan Sandfeld Jensen <kde@carewolf.com>

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

#include <iostream>

#include <alsa/asoundlib.h>
#include <alsa/pcm.h>

#include <audioframe.h>
#include "alsa_sink.h"

namespace aKode {

extern "C" { ALSASinkPlugin alsa_sink; }

struct ALSASink::private_data
{
    private_data() : pcm_playback(0), buffer(0), error(false), can_pause(false) {};

    snd_pcm_t *pcm_playback;

    AudioConfiguration config;
    int scale;
    int filled, fragmentSize;
    int sampleSize;
    char* buffer;
    bool error;
    bool can_pause;
};

ALSASink::ALSASink()
{
    m_data = new private_data;
}

ALSASink::~ALSASink()
{
    close();
    delete m_data;
}

bool ALSASink::open()
{
    int err = 0;
    // open is non-blocking to make it possible to fail when occupied
    err = snd_pcm_open(&m_data->pcm_playback, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
    if (err < 0) {
        m_data->error = true;
        return false;
    }

    // Set to blocking
    snd_pcm_nonblock(m_data->pcm_playback, 0);

    m_data->error = false;
    return true;
}

void ALSASink::close()
{
    if (m_data->pcm_playback) {
        snd_pcm_drain(m_data->pcm_playback);
        snd_pcm_close(m_data->pcm_playback);
    }
    m_data->pcm_playback = 0;
    m_data->error = false;
}

int ALSASink::setAudioConfiguration(const AudioConfiguration* config)
{
    if (m_data->error) return -1;

    // Get back to OPEN state (is SETUP state enough with snd_pcm_drop?)
    snd_pcm_state_t state = snd_pcm_state( m_data->pcm_playback );
    if (state != SND_PCM_STATE_OPEN) {
        close();
        if (!open()) return -1;
    }

    int res = 0;
    int wid = 1;
    m_data->config = *config;
    snd_pcm_hw_params_t *hw;
    snd_pcm_hw_params_alloca(&hw);
    snd_pcm_hw_params_any(m_data->pcm_playback, hw);
    snd_pcm_hw_params_set_access(m_data->pcm_playback, hw, SND_PCM_ACCESS_RW_INTERLEAVED);
    // Detect format:
    snd_pcm_format_t format = SND_PCM_FORMAT_UNKNOWN;
    // Test for float, 24 and 32 bit integer. Fall back to 16bit
    if (m_data->config.sample_width<0) {
        if (snd_pcm_hw_params_test_format(m_data->pcm_playback, hw,  SND_PCM_FORMAT_FLOAT ) == 0) {
            format = SND_PCM_FORMAT_FLOAT;
            m_data->scale = 1;
            wid = 4;
            goto found_format;
        }
        // Try 16bit then
        m_data->config.sample_width = 16;
        res = 1;
    }
    if (m_data->config.sample_width > 24 && m_data->config.sample_width <=32) {
        if (snd_pcm_hw_params_test_format(m_data->pcm_playback, hw, SND_PCM_FORMAT_S32) == 0) {
            format = SND_PCM_FORMAT_S32;
            m_data->scale = 1<<(32-config->sample_width);
            wid = 4;
            goto found_format;
        }
        // Try 24bit then
        m_data->config.sample_width = 24;
        res = 1;
    }
    if (m_data->config.sample_width > 16 && m_data->config.sample_width <= 24) {
        if (snd_pcm_hw_params_test_format(m_data->pcm_playback, hw,  SND_PCM_FORMAT_S24 ) == 0) {
            format = SND_PCM_FORMAT_S24;
            m_data->scale = 1<<(24-config->sample_width);
            wid = 4;
            goto found_format;
        }
        // Try 16bit then
        m_data->config.sample_width = 16;
    }
    // If the driver doesnt support 8 or 16 bit, we will fail completly
    if (m_data->config.sample_width<=8) {
        format = SND_PCM_FORMAT_S8;
        m_data->scale = 1<<(8-config->sample_width);
        wid = 1;
        goto found_format;
    }
    else
    if (m_data->config.sample_width<=16) {
        format = SND_PCM_FORMAT_S16;
        m_data->scale = 1<<(16-config->sample_width);
        wid = 2;
        goto found_format;
    }

found_format:
    if (format != SND_PCM_FORMAT_UNKNOWN)
        snd_pcm_hw_params_set_format(m_data->pcm_playback, hw, format);
    else
        return -1;

    unsigned int rate = config->sample_rate;
    snd_pcm_hw_params_set_rate_near(m_data->pcm_playback, hw, &rate, 0);
    if (m_data->config.sample_rate != rate) {
        m_data->config.sample_rate = rate;
        res = 1;
    }

    snd_pcm_hw_params_set_channels(m_data->pcm_playback, hw, config->channels);


    m_data->fragmentSize = 1024;
    snd_pcm_uframes_t period_size = m_data->fragmentSize / (wid*config->channels);
    snd_pcm_hw_params_set_period_size_near(m_data->pcm_playback, hw, &period_size, 0);

    m_data->fragmentSize = period_size * (wid*config->channels);
//     std::cerr << "akode: ALSA fragment-size: " << m_data->fragmentSize << "\n";

    delete m_data->buffer;
    m_data->buffer = new char [m_data->fragmentSize];
    m_data->filled = 0;

    if (snd_pcm_hw_params(m_data->pcm_playback, hw) < 0) {
        return -1;
    }
    else {
        m_data->can_pause = (snd_pcm_hw_params_can_pause(hw) == 1);
        return res;
    }
}

const AudioConfiguration* ALSASink::audioConfiguration() const
{
    return &m_data->config;
}

template<class T>
bool ALSASink::_writeFrame(AudioFrame* frame)
{
    int channels = m_data->config.channels;

    long i = 0;
    T* buffer = (T*)m_data->buffer;
    T** data = (T**)frame->data;
    while(true) {
        if (m_data->filled >= m_data->fragmentSize)
        xrun:
        {
            int frames = snd_pcm_bytes_to_frames(m_data->pcm_playback, m_data->filled);
            int status = snd_pcm_writei(m_data->pcm_playback, m_data->buffer, frames);
            if (status == -EPIPE) {
                snd_pcm_prepare(m_data->pcm_playback);
                std::cerr << "akode: ALSA xrun\n";
                goto xrun;
            }
            else if (status < 0) return false;
            int bytes = snd_pcm_frames_to_bytes(m_data->pcm_playback, status);
            if (m_data->filled != bytes) {
                int rest = m_data->filled - bytes;
                std::cerr << "akode: ALSA write-remainder: " << rest << "\n";
                memmove(m_data->buffer, m_data->buffer + bytes, rest);
                m_data->filled = rest;
            } else
                m_data->filled = 0;

        }
        if (i >= frame->length) break;
        for(int j=0; j<channels; j++) {
            buffer[m_data->filled/sizeof(T)] = (data[j][i])*m_data->scale;
            m_data->filled+=sizeof(T);
        }
        i++;
    }

    if (snd_pcm_state( m_data->pcm_playback ) == SND_PCM_STATE_PREPARED)
        snd_pcm_start(m_data->pcm_playback);

    return true;
}

bool ALSASink::writeFrame(AudioFrame* frame)
{
    if (m_data->error) return false;

    if ( frame->sample_width != m_data->config.sample_width
      || frame->channels != m_data->config.channels
      || frame->sample_rate != m_data->config.sample_rate)
    {
        if (setAudioConfiguration(frame) < 0)
            return false;
    }

    if (snd_pcm_state( m_data->pcm_playback ) == SND_PCM_STATE_SETUP)
        snd_pcm_prepare(m_data->pcm_playback);

    if (frame->sample_width<0)
        return _writeFrame<float>(frame);
    else
    if (frame->sample_width<=8)
        return _writeFrame<int8_t>(frame);
    else
    if (frame->sample_width<=16)
        return _writeFrame<int16_t>(frame);
    else
    if (frame->sample_width<=32)
        return _writeFrame<int32_t>(frame);

    return false;
}

void ALSASink::pause()
{
    if (m_data->error) return;

    if (m_data->can_pause) {
        snd_pcm_pause(m_data->pcm_playback, 1);
    }

}

// Do not confuse this with snd_pcm_resume which is used to resume from a suspend
void ALSASink::resume()
{
    if (m_data->error) return;

    if (snd_pcm_state( m_data->pcm_playback ) == SND_PCM_STATE_PAUSED)
        snd_pcm_pause(m_data->pcm_playback, 0);
}

} // namespace
