/*  aKode: Cross-fader

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
#ifndef _AKODE_CROSSFADER_H
#define _AKODE_CROSSFADER_H

namespace aKode {

class AudioFrame;

class CrossFader {
    int time;
    int pos;
    AudioFrame source;
public:
    // Set number of milliseconds the crosslap should last
    CrossFader(unsigned int time = 100);
    bool writeFrame(AudioFrame* frame);
    bool doFrame(AudioFrame* frame);
    bool readFrame(AudioFrame* frame);
    void setLength(unsigned int time);
    bool full();
    bool done();
};

} // namespace

#endif
