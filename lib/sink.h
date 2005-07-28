/*  aKode: Sink abstract-type

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

#ifndef _AKODE_SINK_H
#define _AKODE_SINK_H

#include "pluginhandler.h"

#include "akode_export.h"

namespace aKode {

class AudioFrame;
class AudioBuffer;
class AudioConfiguration;


//! A generic interface for all sinks

/*!
 * Sinks are where an audiostream goes. It can in aKodeLib be either an
 * encoder or an audio-output such as ALSA or OSS.
 */
class AKODE_EXPORT Sink {
public:
    virtual ~Sink() {};
    /*!
     * Opens the sinks and returns true if succesfull.
     */
    virtual bool open() = 0;
    /*!
     * Tries to configure the sink for the configuration \a config.
     * Returns 0 if a perfect match was possible, positive if a close
     * match was possible, and negative to indicate complete failure.
     */
    virtual int setAudioConfiguration(const AudioConfiguration* config) = 0;
    /*!
     * Returns the configuration the sink currently expects.
     */
    virtual const AudioConfiguration* audioConfiguration() const = 0;
    /*!
     * Writes one frame to the sink. If the frame has a different configuration
     * than the sink expects, the result is undefined.
     *
     * Some sinks might handle automatic reconfiguration gracefully, but this
     * will be plugin-specific and cannot be generally relied upon.
     */
    virtual bool writeFrame(AudioFrame* frame) = 0;
};

class SinkPlugin {
public:
    /*!
     * Asks the plugin to open a Sink, returns 0 if the
     * plugin could not.
     */
    virtual Sink* openSink() = 0;
};

class AKODE_EXPORT SinkPluginHandler : public PluginHandler, public SinkPlugin {
public:
    static list<string> listSinkPlugins();

    SinkPluginHandler() : sink_plugin(0) {};
    SinkPluginHandler(const string name);
    Sink* openSink();
    /*!
     * Loads the sink-plugin named \a name (alsa,oss..)
     */
    virtual bool load(const string name);
    virtual void unload();
    bool isLoaded() { return sink_plugin != 0; };
protected:
    SinkPlugin* sink_plugin;
};

} // namespace

#endif
