/*  aKode: SinkPluginHandler

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

#include "akodelib.h"
#include "sink.h"
#include "auto_sink.h"

namespace aKode {

list<string> SinkPluginHandler::listSinkPlugins() {
    const list<string> plugin_list = listPlugins();
    list<string> sink_list;
    for (list<string>::const_iterator i = plugin_list.begin(); i != plugin_list.end(); i++)
        if (i->length() > 5 && i->substr(i->length()-5,5) == "_sink")
            sink_list.push_back(i->substr(0,i->length()-5));
    return sink_list;
}


SinkPluginHandler::SinkPluginHandler(const string lib) : sink_plugin(0)
{
    if (lib.size() > 0);
        load(lib);
}

bool SinkPluginHandler::load(const string name)
{
    if (library_loaded) return false;

    bool res = PluginHandler::load(name+"_sink");
    if (res)
        sink_plugin = (SinkPlugin*)loadPlugin(name+"_sink");
    else
        if (name == "auto")
            sink_plugin = &auto_sink;
        else
            return false; // plugin not found

    if (!res)
        unload();

    return res;
}

Sink* SinkPluginHandler::openSink() {
    if (sink_plugin)
        return sink_plugin->openSink();
    else
        return 0;
}

} //namespace