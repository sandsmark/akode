/*  aKode AudioFrame

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

#ifndef _AKODE_AUDIOFRAME_H
#define _AKODE_AUDIOFRAME_H

#include "akodelib.h"
#include "audioconfiguration.h"

namespace aKode {

#define AKODE_POS_UNKNOWN -1
#define AKODE_POS_EOF -2
#define AKODE_POS_ERROR -3

//! The internal audio format

/*!
 * AudioFrames are used through-out akodelib as the mean of audiotransport.
 * It derives from AudioConfiguration because it caries its own interpretation
 * around with it.
 */
struct AudioFrame : public AudioConfiguration {
public:
    AudioFrame() : length(0), max(0), data(0) {};
    ~AudioFrame() { freeSpace(); }
    /*!
     * Reserves space in the frame for atleast \a iLength samples of the
     * configuration \a config.
     */
    void reserveSpace(const AudioConfiguration *config, long iLength) {
        reserveSpace(config->channels, iLength, config->sample_width);
        sample_rate = config->sample_rate;
        channel_config = config->channel_config;
        surround_config = config->surround_config;
    }
    void reserveSpace(uint8_t iChannels, long iLength, int8_t iWidth) {
        if ( data != 0 && channels == iChannels &&
             max >= iLength && sample_width == iWidth)
        {    // Fast common case
            length = iLength;
            return;
        }
	freeSpace();
	channels = iChannels;
	length = max = iLength;
	sample_width = iWidth;
	data = new int8_t*[channels+1];
	int bytes = (sample_width+7) / 8;
        if (bytes > 2) bytes = 4;
        if (bytes < 0) bytes = 4;
	for(int i=0; i<iChannels; i++)
	    data[i] = new int8_t[length*bytes];
	data[iChannels] = 0;
    }
    /*!
     * Frees the space allocated for the buffer.
     */
    void freeSpace()
    {
        if (!data) return;
	int8_t** tmp = data;
	while(*tmp) {
	   delete[] *tmp;
	   tmp++;
	}
	delete[] data;
        pos = 0;
	data = 0;
	channels = 0;
	length = 0;
        max = 0;
    }
    /*!
     * The current position in stream (measured in milliseconds)
     * -1 if unknown
     */
    long pos;
    /*!
     * The length of the frame in samples.
     */
    long length;
    /*!
     * The maximum number of samples currently reserved in the frame.
     */
    long max;
    /*!
     * The buffer is accessed according sample-width
     * 1-8bit: int8_t, 9-16bit: int16_t, 17-32bit: int32_t
     * -32bit: float
     */
    int8_t** data;
};

} // namespace

#endif
