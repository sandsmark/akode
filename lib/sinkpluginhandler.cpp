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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include <iostream>

#include "akodelib.h"
#include "sink.h"
#include "auto_sink.h"
#include "void_sink.h"

namespace aKode {

list<string> SinkPluginHandler::listSinkPlugins() {
    const list<string> plugin_list = listPlugins();
    list<string> sink_list;
    for (list<string>::const_iterator i = plugin_list.begin(); i != plugin_list.end(); i++)
        if (i->length() > 5 && i->substr(i->length()-5,5) == "_sink")
            sink_list.push_back(i->substr(0,i->length()-5));
    sink_list.push_back("auto");
    sink_list.push_back("void");
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
        if (name == "void") {
            sink_plugin = &void_sink;
        } else {
            std::cerr << "Plugin not found\n";
            return false; // plugin not found
        }

    return res;
}

void SinkPluginHandler::unload() {
    sink_plugin = 0;
    PluginHandler::unload();
}

Sink* SinkPluginHandler::openSink() {
    if (sink_plugin)
        return sink_plugin->openSink();
    else
        return 0;
}

} //namespace
