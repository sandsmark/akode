/*  aKode: EncopderPluginHandler

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

#include "akodelib.h"
#include "encoder.h"

namespace aKode {

EncoderPluginHandler::EncoderPluginHandler(const string lib) : encoder_plugin(0)
{
    if (lib.size() > 0)
        load(lib);
}


bool EncoderPluginHandler::load(const string name)
{
    bool res = PluginHandler::load(name+"_encoder");
    if (res)
        encoder_plugin = (EncoderPlugin*)loadPlugin(name+"_encoder");

    return res && encoder_plugin;
}

Encoder* EncoderPluginHandler::openEncoder(File *dest)
{
    if (encoder_plugin)
        return encoder_plugin->openEncoder(dest);
    else
        return 0;
}

} //namespace
