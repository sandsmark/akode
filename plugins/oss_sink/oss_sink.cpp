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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <config.h>

#if defined(HAVE_SYS_SOUNDCARD_H)
#include <sys/soundcard.h>
#elif defined(HAVE_SOUNDCARD_H)
#include <soundcard.h>
#endif

#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <audioframe.h>
#include "oss_sink.h"

//#include <iostream>

namespace aKode {

extern "C" { OSSSinkPlugin oss_sink; }

struct OSSSink::private_data
{
    private_data() : device(0) {};
    int audio_fd;

    const char *device;
    AudioConfiguration config;
    int scale;
    bool valid;
};


static const char *_devices[] = {
    "/dev/dsp",
    "/dev/sound/dsp0",
    "/dev/audio",
    0
};

OSSSink::OSSSink()
{
    m_data = new private_data;
}

OSSSink::~OSSSink()
{
    close();
    delete m_data;
}

bool OSSSink::open()
{
    const char** device = _devices;
    while (*device) {
        if(::access(*device, F_OK) == 0) {
            m_data->device = *device;
            break;
        }
        device++;
    }

    if (!m_data->device) goto failed;

    m_data->audio_fd = ::open(m_data->device, O_WRONLY, 0);

    if (m_data->audio_fd == -1) goto failed;
    m_data->valid = true;
    return true;

failed:
    m_data->valid = false;
    return false;
}


void OSSSink::close() {
    if (m_data->valid) ::close(m_data->audio_fd);
    m_data->valid = false;
}

int OSSSink::setAudioConfiguration(const AudioConfiguration* config)
{
    m_data->config = *config;

    int format = AFMT_S16_NE; // 16bit native endian

    ioctl(m_data->audio_fd, SNDCTL_DSP_SETFMT, &format);

    if (format != AFMT_S16_NE) return -1;
    m_data->scale = 16-config->sample_width;

    int stereo;
    if (config->channels == 1)
        stereo = 0;
    else
        stereo = 1;

    ioctl(m_data->audio_fd, SNDCTL_DSP_STEREO, &stereo);

    m_data->config.channel_config = MonoStereo;
    if (stereo == 0)
        m_data->config.channels = 1;
    else
        m_data->config.channels = 2;

    ioctl(m_data->audio_fd, SNDCTL_DSP_SPEED, &m_data->config.sample_rate);

    return 1;
}

const AudioConfiguration* OSSSink::audioConfiguration() const
{
    return &m_data->config;
}

bool OSSSink::writeFrame(AudioFrame* frame)
{
    if (!m_data->valid) return false;

    if ( frame->sample_width != m_data->config.sample_width
      || frame->channels != m_data->config.channels )
    {
        if (setAudioConfiguration(frame) < 0)
            return false;
    }

    int channels = m_data->config.channels;
    int length = frame->length;

    int16_t *buffer = (int16_t*)alloca(length*channels*2);
    int16_t** data = (int16_t**)frame->data;
    for(int i = 0; i<length; i++)
        for(int j=0; j<channels; j++)
            buffer[i*channels+j] = data[j][i]<<m_data->scale;

//    std::cerr << "Writing frame\n";
    int status = 0;
    do {
        status = ::write(m_data->audio_fd, buffer, channels*length*2);
        if (status == -1) {
//            if (errno == EAGAIN) continue;
            if (errno == EINTR) continue;
            return false;
        }
    } while(false);

    return true;
}

} // namespace
