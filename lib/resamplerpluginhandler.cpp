/*  aKode: ResamplerPluginHandler

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

#include "akodelib.h"
#include "resampler.h"
#include "fast_resampler.h"

namespace aKode {

ResamplerPluginHandler::ResamplerPluginHandler(const string lib) : resampler_plugin(0)
{
    if (lib.size() > 0)
        load(lib);
}

bool ResamplerPluginHandler::load(const string name)
{
    bool res = PluginHandler::load(name+"_resampler");
    if (res)
        resampler_plugin = (ResamplerPlugin*)loadPlugin(name+"_resampler");
    else
        if (name == "fast")
            resampler_plugin = &fast_resampler;

    return res && resampler_plugin;
}

Resampler* ResamplerPluginHandler::openResampler() {
    return resampler_plugin->openResampler();
}

} //namespace
