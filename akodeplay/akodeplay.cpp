/*  aKode-utils: akodeplay

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

#include <iostream>

#include "../lib/localfile.h"
#include "../lib/audiobuffer.h"
#include "../lib/audioframe.h"
#include "../lib/resampler.h"
#include "../lib/decoder.h"
#include "../lib/sink.h"
#include "../lib/buffered_decoder.h"

#include <getopt.h>

using namespace std;
using namespace aKode;

void usage() {
    cout << "Usage: akodeplay [-s sink] [-r resampler] -d decoder filename" << endl;
};

void list_sinks() {
    cout << "Available sinks:" << endl;
    list<string> sinks = SinkPluginHandler::listSinkPlugins();
    for(list<string>::const_iterator s = sinks.begin(); s != sinks.end(); s++)
        cout << "\t" << *s << endl;
};

void list_decoders() {
    cout << "Available decoders:" << endl;
    list<string> plugins = DecoderPluginHandler::listDecoderPlugins();
    for(list<string>::const_iterator s = plugins.begin(); s != plugins.end(); s++)
        cout << "\t" << *s << endl;
};

static struct option longoptions[] = {
    {"resampler", 1, 0, 'r'},
    {"decoder", 1, 0, 'd'},
    {"sink", 1, 0, 's'},
    {0, 0, 0, 0}
};

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
    while ((opt = getopt_long(argc, argv,  "r:d:s:", longoptions, 0)) != -1) {
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

    if (!sink_plugin) sink_plugin = "auto";
    if (!decoder_plugin) {
        usage();
        exit(1);
    }
    if (!resampler_plugin) resampler_plugin = "fast";

    filename = argv[::optind];

    aKode::File *file = new aKode::LocalFile(filename);
    if (!file->openRO()) {
        cerr << "Unreadable file: " << filename << endl;
        exit(1);
    } else
        file->close();

    cout << "Opening decoder for " << filename << endl;
    Decoder *decoder;
    Resampler *resampler;
    Sink *sink;

    ResamplerPluginHandler resamplerPlugin(resampler_plugin);
    DecoderPluginHandler decoderPlugin(decoder_plugin);
    SinkPluginHandler sinkPlugin(sink_plugin);

    if (!decoderPlugin.isLoaded()) {
        cout << "Could not load decoder-plugin: " << decoder_plugin << endl;
        list_decoders();
        exit(1);
    }

    if (!sinkPlugin.isLoaded()) {
        cout << "Could not load sink-plugin: " << sink_plugin << endl;
        list_sinks();
        exit(1);
    }

    if (!resamplerPlugin.isLoaded()) {
        cout << "Could not load resampler-plugin: " << resampler_plugin << endl;
        exit(1);
    }

    resampler = resamplerPlugin.openResampler();
    decoder = decoderPlugin.openDecoder(file);
    sink = sinkPlugin.openSink();

    if (!sink->open()) {
        cout << "Could not open sink-plugin: " << sink_plugin << endl;
        exit(1);
    }

    if (!decoder) {
        cout << "Could not open decoder-plugin: " << decoder_plugin << endl;
        exit(1);
    }

    aKode::BufferedDecoder *bufdecoder = new aKode::BufferedDecoder();
    bufdecoder->openDecoder(decoder);
    // starts buffering
    bufdecoder->start();
    //FrameDecoder *fdecoder = decoder;

    aKode::AudioFrame *inFrame, *outFrame;
    inFrame = new aKode::AudioFrame;
    outFrame = inFrame;

    // Get first frame
    while(!bufdecoder->readFrame(inFrame)) {
        if (bufdecoder->error()) {
            cout << "Invalid format\n";
            exit(2);
        }
    }
    {
        cout << "Trying to set sink to: \n";
        cout << "Channels: " << (int)inFrame->channels << "\n";
        cout << "Sample-rate(width): " << inFrame->sample_rate << "(" << (int)inFrame->sample_width << ")\n";
    }

    if (sink->setAudioConfiguration(inFrame)<0) {
        cout << "Configuration of sink failed\n";
        exit(1);
    }
    const AudioConfiguration *got = sink->audioConfiguration();
    {
        cout << "Got configuration: \n";
        cout << "Channels: " << (int)got->channels << "\n";
        cout << "Sample-rate(width): " << got->sample_rate << "(" << (int)got->sample_width << ")\n";
    }

    AudioFrame reFrame;
    if (inFrame->sample_rate != got->sample_rate) {
        cout << "Resampling to: " << got->sample_rate <<"\n";
        outFrame = &reFrame;
        resampler->setSampleRate(got->sample_rate);
        resampler->doFrame(inFrame, outFrame);
    }

    sink->writeFrame(outFrame);

    while (!bufdecoder->eof() && !bufdecoder->error()) {
        while(bufdecoder->readFrame(inFrame)) {
            if (inFrame->sample_rate != got->sample_rate) {
                outFrame = &reFrame;
                resampler->doFrame(inFrame,outFrame);
            } else
                outFrame = inFrame;
            sink->writeFrame(outFrame);
        };
        //cerr << "Frame error\n";
    }

    bufdecoder->stop();
    bufdecoder->closeDecoder();
    delete bufdecoder;
    delete decoder;
    if (inFrame != outFrame)
        delete outFrame;
    delete inFrame;
    delete file;
    return 0;
}
