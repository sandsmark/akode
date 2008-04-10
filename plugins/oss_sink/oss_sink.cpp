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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
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

#include <iostream>

namespace aKode {

extern "C" { OSSSinkPlugin oss_sink; }

struct OSSSink::private_data
{
    private_data() : audio_fd(-1), device(0), valid(false), buffer(0), buffer_length(0) {};
    int audio_fd;

    const char *device;
    AudioConfiguration config;
    bool valid;

    char* buffer;
    int buffer_length;
};


static const char *_devices[] = {
    "/dev/dsp",
    "/dev/sound/dsp0",
    "/dev/audio",
    0
};

OSSSink::OSSSink()
{
    d = new private_data;
}

OSSSink::~OSSSink()
{
    close();
    delete d;
}

bool OSSSink::open()
{
    const char** device = _devices;
    while (*device) {
        if(::access(*device, F_OK) == 0) {
            break;
        }
        device++;
    }

    if (!*device) {
        std::cerr << "akode: No OSS device found\n";
        d->valid = false;
        return false;
    }

    return openDevice(*device);
}

bool OSSSink::openDevice(const char *device)
{
    d->device = device;

    // Set non-blocking to not block on open
    d->audio_fd = ::open(d->device, O_WRONLY | O_NONBLOCK, 0);

    if (d->audio_fd == -1) {
        std::cerr << "akode: Could not open " << d->device << " for writing\n";
        goto failed;
    }
    // set blocking again to block on write
    fcntl(d->audio_fd, F_SETFL, O_WRONLY);
    d->valid = true;
    return true;

failed:
    d->valid = false;
    return false;
}


void OSSSink::close() {
    if (d->audio_fd != -1) ::close(d->audio_fd);
    d->audio_fd = -1;
    delete d->buffer;
    d->buffer = 0;
    d->buffer_length = 0;
    d->valid = false;
}

int OSSSink::setAudioConfiguration(const AudioConfiguration* config)
{
    d->config = *config;

    int format = 0;

    if (config->sample_width > 0 && config->sample_width <= 8)
        format = AFMT_S8;
    else
        format = AFMT_S16_NE; // 16bit native endian

    ioctl(d->audio_fd, SNDCTL_DSP_SETFMT, &format);

    if (format == AFMT_S16_NE)
        d->config.sample_width = 16;
    else
    if (format == AFMT_S8)
        d->config.sample_width = 8;
    else
        return -1;

    int stereo;
    if (config->channels == 1)
        stereo = 0;
    else
        stereo = 1;

    ioctl(d->audio_fd, SNDCTL_DSP_STEREO, &stereo);

    d->config.channel_config = MonoStereo;
    if (stereo == 0)
        d->config.channels = 1;
    else
        d->config.channels = 2;

    ioctl(d->audio_fd, SNDCTL_DSP_SPEED, &d->config.sample_rate);

    return 1;
}

const AudioConfiguration* OSSSink::audioConfiguration() const
{
    return &d->config;
}

bool OSSSink::writeFrame(AudioFrame* frame)
{
    if (!d->valid) return false;

    if ( frame->sample_width != d->config.sample_width
      || frame->channels != d->config.channels )
    {
        if (setAudioConfiguration(frame) < 0)
            return false;
    }

    int channels = d->config.channels;
    int length = frame->length;
    int bytelen = length*channels*((d->config.sample_width+7)/8);

    if (bytelen > d->buffer_length) {
        delete d->buffer;
        d->buffer = new char[bytelen];
        d->buffer_length = bytelen;
    }

    if (d->config.sample_width == 8) {
        int8_t *buffer = (int8_t*)d->buffer;
        int8_t** data = (int8_t**)frame->data;
        for(int i=0; i<length; i++)
            for(int j=0; j<channels; j++)
                buffer[i*channels+j] = data[j][i];
    } else {
        int16_t *buffer = (int16_t*)d->buffer;
        int16_t** data = (int16_t**)frame->data;
        for(int i=0; i<length; i++)
            for(int j=0; j<channels; j++)
                buffer[i*channels+j] = data[j][i];
    }

    int status = 0;
    do {
        status = ::write(d->audio_fd, d->buffer, bytelen);
        if (status == -1) {
//            if (errno == EAGAIN) continue;
            if (errno == EINTR) continue;
            return false;
        }
    } while(false);

    return true;
}

} // namespace
