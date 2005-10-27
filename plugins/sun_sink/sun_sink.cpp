/*  aKode: Sun-Sink

    Copyright (C) 2005 Allan Sandfeld Jensen <kde@carewolf.com>

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

#include <config.h>

#include <sys/audioio.h>

#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <alloca.h>

#include <audioframe.h>
#include "sun_sink.h"

//#include <iostream>

namespace aKode {

extern "C" { SunSinkPlugin oss_sink; }

struct SunSink::private_data
{
    private_data() : audio_fd(-1), device(0) {};
    int audio_fd;
    audio_info_t auinfo;

    const char *device;
    AudioConfiguration config;
    bool valid;
};


SunSink::SunSink()
{
    d = new private_data;
}

SunSink::~SunSink()
{
    close();
    delete d;
}

bool SunSink::open()
{
    const char* device = getenv("AUDIODEV");
    do {
        if (device && ::access(device, F_OK) == 0) break;
        device = "/dev/audio";
        if (::access(device, F_OK) == 0) break;
        goto failed;
    } while (false);
    d->device = device;

    d->audio_fd = ::open(d->device, O_WRONLY, 0);

    if (d->audio_fd == -1) goto failed;
    d->valid = true;
    return true;

failed:
    d->valid = false;
    return false;
}


void SunSink::close() {
    if (d->audio_fd != -1) ::close(d->audio_fd);
    d->audio_fd = -1;
    d->valid = false;
}

int SunSink::setAudioConfiguration(const AudioConfiguration* config)
{
    d->config = *config;

    AUDIO_INITINFO(&d->auinfo)

    if (ioctl(d->audio_fd, AUDIO_GETINFO, &d->auinfo) < 0)
    {
        d->valid = false;
        return -1;
    }

    int width = config->sample_width;
    if (width < 0 ) width = 16;
    d->auinfo.play.precision = width;

    d->auinfo.play.encoding = AUDIO_ENCODING_LINEAR;

    d->auinfo.play.channels = config->channels;
    d->auinfo.play.sample_rate = config->sample_rate;

    if (ioctl(d->audio_fd, AUDIO_SETINFO, &d->auinfo) < 0)
    {
        d->valid = false;
        return -1;
    }

    if (ioctl(d->audio_fd, AUDIO_GETINFO, &d->auinfo) < 0)
    {
        d->valid = false;
        return -1;
    }

    d->config.sample_width = d->auinfo.play.precision;
    d->config.channels = d->auinfo.play.channels;
    d->config.sample_rate = d->auinfo.play.sample_rate;
    if (d->config.channels <= 2) d->config.channel_config = MonoStereo;

    if (d->config == *config)
        return 0;
    else
        return 1;
}

const AudioConfiguration* SunSink::audioConfiguration() const
{
    return &d->config;
}

bool SunSink::writeFrame(AudioFrame* frame)
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

    int16_t *buffer = (int16_t*)alloca(length*channels*2);
    int16_t** data = (int16_t**)frame->data;
    for(int i = 0; i<length; i++)
        for(int j=0; j<channels; j++)
            buffer[i*channels+j] = data[j][i];

//    std::cerr << "Writing frame\n";
    int status = 0;
    do {
        status = ::write(d->audio_fd, buffer, channels*length*2);
        if (status == -1) {
//            if (errno == EAGAIN) continue;
            if (errno == EINTR) continue;
            return false;
        }
    } while(false);

    return true;
}

} // namespace
