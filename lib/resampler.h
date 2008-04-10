/*  aKode: Resampler abstract-type

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

#ifndef _AKODE_RESAMPLER_H
#define _AKODE_RESAMPLER_H

#include <string>

#include "pluginhandler.h"
#include "akode_export.h"

namespace aKode {

using std::string;
class AudioFrame;

//! A generic interface for all resamplers

/*!
 * Resamplers are used to change the samplerate of sample
 */
class AKODE_EXPORT Resampler {
public:
    virtual ~Resampler() {};
    /*!
     * Returns true if convertion was succesfull.
     */
    virtual bool doFrame(AudioFrame *in, AudioFrame *out) = 0;
    /*!
     * Sets the output sample-rate to \a value.
     */
    virtual void setSampleRate(unsigned int rate) = 0;
    /*!
     * Sets the resample speed to \a value.
     */
    virtual void setSpeed(float value) = 0;
};

class ResamplerPlugin {
public:
    virtual Resampler* openResampler() = 0;
};

class AKODE_EXPORT ResamplerPluginHandler : public PluginHandler, public ResamplerPlugin {
public:
    ResamplerPluginHandler() : resampler_plugin(0) {};
    ResamplerPluginHandler(const string name);
    Resampler* openResampler();
    /*!
     * Loads a resampler-plugin by the \a name (src, fast..)
     */
    virtual bool load(const string name);
    bool isLoaded() { return resampler_plugin != 0; };
protected:
    ResamplerPlugin* resampler_plugin;
};

} // namespace

#endif
