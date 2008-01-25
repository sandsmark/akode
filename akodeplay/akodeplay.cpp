/*  aKode-utils: akodeplay

    Copyright (C) 2005 Allan Sandfeld Jensen <kde@carewolf.com>

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

#include <cstdlib>
#include <iostream>
#include <cstdlib>

#include "../lib/akodelib.h"
#include "../lib/player.h"
#include "../lib/sink.h"
#include "../lib/decoder.h"

#ifdef HAVE_GNU_GETOPT
  #include <getopt.h>
#else
  #include <unistd.h>
#endif

using namespace std;
using namespace aKode;

void usage() {
    cout << "Usage: akodeplay [-s sink] [-r resampler] [-d decoder] filenames.." << endl;
}

void list_sinks() {
    cout << "Available sinks:" << endl;
    list<string> sinks = SinkPluginHandler::listSinkPlugins();
    for(list<string>::const_iterator s = sinks.begin(); s != sinks.end(); s++)
        cout << "\t" << *s << endl;
}

void list_decoders() {
    cout << "Available decoders:" << endl;
    list<string> plugins = DecoderPluginHandler::listDecoderPlugins();
    for(list<string>::const_iterator s = plugins.begin(); s != plugins.end(); s++)
        cout << "\t" << *s << endl;
}

#ifdef HAVE_GNU_GETOPT
static struct option longoptions[] = {
    {"resampler", 1, 0, 'r'},
    {"decoder", 1, 0, 'd'},
    {"sink", 1, 0, 's'},
    {0, 0, 0, 0}
};
#endif

int main(int argc, char** argv) {
    const char* resampler_plugin = 0;
    const char* decoder_plugin = 0;
    const char* sink_plugin = 0;
    const char* filename = 0;

    if (argc <= 1) {
        usage();
        exit(1);
    }

    int opt;
#ifdef HAVE_GNU_GETOPT
    while ((opt = getopt_long(argc, argv,  "r:d:s:", longoptions, 0)) != -1)
#else
    while ((opt = getopt(argc, argv,  "r:d:s:")) != -1)
#endif
    {
        switch (opt) {
            case 'r':
                resampler_plugin = ::optarg;
                continue;
            case 'd':
                decoder_plugin = ::optarg;
                continue;
            case 's':
                sink_plugin = ::optarg;
                continue;
            default:
                usage();
                exit(1);
        }
    }

    aKode::Player player;

    if (!sink_plugin) sink_plugin = "auto";

    if (decoder_plugin)
        player.setDecoderPlugin(decoder_plugin);

    if (resampler_plugin)
        player.setResamplerPlugin(resampler_plugin);

    if (!player.open(sink_plugin)) {
        cout << "Could not load sink-plugin: " << sink_plugin << endl;
        list_sinks();
        exit(1);
    }

    while (::optind < argc) {
        filename = argv[::optind++];

        if (!player.load(filename)) {
            cout << "Could not load file: " << filename << endl;
            exit(1);
        }

        player.play();
        player.wait();

        player.unload();
    }
    player.close();

    return 0;
}
