/*  aKode AudioConfiguration

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

#ifndef _AKODE_AUDIOCONFIGURATION_H
#define _AKODE_AUDIOCONFIGURATION_H

#include "akodelib.h"

namespace aKode {

enum ChannelConfiguration {
	MultiChannel = 0,  // any number of independent channels
	MonoStereo = 1,    // 1-2 channels
	Surround = 2 	   // 1-7 channels
	// Two extra flags will make it here sometime
	// One for Dolby Pro (make 3 front channels out of 2)
	// and Dolby Digital ES (make an extra back channel from the two surround)
	// and one for Dolby Surround (make 4 channels surround out of stereo).
};

struct SurroundConfiguration {
    SurroundConfiguration() : front_channels(0)
                            , rear_channels(0)
                            , side_channels(0)
                            , LFE_channel(0)
                            , reserved(0) {};
    SurroundConfiguration(uint8_t val) { *(uint8_t*)(this) = val; };
    uint8_t front_channels : 3; // 1-5
    uint8_t rear_channels : 2; // 0-3
    uint8_t side_channels : 1; // 0,2
    uint8_t LFE_channel : 1;
    uint8_t reserved : 1;
    operator uint8_t() const {
        return *(uint8_t*)(this);
    }
};
// Channel ordering:
// front(, rear)(, side)(, LFE)
// (left, right(,)) (center)(, left-of-center, right-of-center)
//
// front channels:
// 1 -> center
// 2 -> left, right
// 3 -> left, right, center
// 4 -> left, right, left-of-center, right-of-center
// 5 -> left, right, center, left-of-center, right-of-center
//
// rear channels:
// 1 -> center
// 2 -> left, right
// 3 -> left, right, center

// 8 bytes of configuration
struct AudioConfiguration {
public:
    AudioConfiguration() : channels(0),
                           channel_config(0),
                           surround_config(0),
                           sample_width(0),
                           sample_rate(0) {};
    // 0-255 channels (0 means no audio)
    uint8_t channels;
    // enum ChannelConfiguration
    uint8_t channel_config;
    // struct SurroundConfiguration
    uint8_t surround_config;
    // 8, 16, 24 and 32 for signed integer samples
    // -32 for 32bit float
    int8_t sample_width;
    // Sample-rate in Hz
    uint32_t sample_rate;

    bool operator==(const AudioConfiguration& b) const
    {
        // abusing that we are only 8 bytes long
        const uint32_t *you = (const uint32_t*)(&b);
        const uint32_t *me = (const uint32_t*)(this);

        return (you[0] == me[0]) && (you[1] == me[1]);
    }
};

} // namespace

#endif
