/*  aKode: DecoderPluginHandler

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
#include "decoder.h"
#include "wav_decoder.h"

namespace aKode {

list<string> DecoderPluginHandler::listDecoderPlugins() {
    const list<string> plugin_list = PluginHandler::listPlugins();
    list<string> decoder_list;
    for (list<string>::const_iterator i = plugin_list.begin(); i != plugin_list.end(); i++)
        if (i->length() > 8 && i->substr(i->length()-8,8) == "_decoder")
            decoder_list.push_back(i->substr(0,i->length()-8));
    return decoder_list;
}

DecoderPluginHandler::DecoderPluginHandler(const string lib) : decoder_plugin(0)
{
    if (lib.size() > 0)
        load(lib);
}

bool DecoderPluginHandler::load(const string name)
{
    if (library_loaded) return false;

    bool res = PluginHandler::load(name+"_decoder");
    if (res)
        decoder_plugin = (DecoderPlugin*)loadPlugin(name+"_decoder");
    else
        if (name == "wav") {
            decoder_plugin = &wav_decoder;
            res = true;
        } else
            return false;

    return res;
}

void DecoderPluginHandler::unload() {
    decoder_plugin = 0;
    PluginHandler::unload();
}

Decoder* DecoderPluginHandler::openDecoder(File *src) {
    if (decoder_plugin)
        return decoder_plugin->openDecoder(src);
    else
        return 0;
}

} //namespace
