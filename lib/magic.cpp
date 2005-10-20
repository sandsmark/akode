/*  aKode: Magic

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
#include "magic.h"
#include "file.h"
#include <iostream>
using std::cerr;

namespace aKode {
    namespace Magic {

    static int detectID3v2(File *src)
    {
        char header[6];
        unsigned char *buf = (unsigned char*)header;
        if(src->read(header, 4)) {
            // skip id3v2 headers
            if (memcmp(header, "ID3", 3) == 0) {
                src->read(header, 6);
                int size = 10;
                if (buf[1] & 0x10) size += 10;
                if (buf[5] > 127 || buf[4] > 127 || buf[3] > 127 || buf[2] > 127)
                {
                    size += buf[5];
                    size += buf[4] << 8;
                    size += buf[3] << 18;
                    size += buf[2] << 24;
    	       	    cerr << "Un-unsynchronized size\n";
                }
                size += buf[5];
                size += buf[4] << 7;
                size += buf[3] << 14;
                size += buf[2] << 21;
// 	    	cerr << "ID3v2 header found(size="<< size << ")\n";
                return size;
            }
        }
        return 0;
    }
/*
    struct OggMagic {
        char magic[8];
        int len;
        void *format;
    }

    OggMagic ogg_magic = {
        {"fLaC",       4, &oggflac_format},
        {"\x7fFLAC",   5, &oggflac_format},
        {"\x01vorbis", 7, &vorbis_format},
        {"Speex  ",    7, &speex_format}
        {"",           0, 0}
    }

    Format* detectOgg(File *src, int skip) {
        Format *res = 0;
        char oggmagic[7];
        src->seek(skip+28);
        src->read(oggmagic, 7);

        for (OggMagic *i = ogg_magic; i->len >0; i++) {
            if (memcmp(oggmagic, i->magic, i->len) == 0)
                return i->format;
        }

        return res;
    } */

    string detectRIFF(File *src, int skip) {
        string res;
        char riffmagic[4];
        src->seek(skip+8);
        src->read(riffmagic, 4);
        if (memcmp(riffmagic, "WAVE",4) == 0) {
            char wavmagic[2];
            src->seek(skip+20);
            src->read(wavmagic, 2);
            if (wavmagic[0] == 1)
                res = "wav";
            else
            if (wavmagic[0] == 80)
                res = "mpeg";
            else
            if (wavmagic[0] == 85)
                res = "mpeg";
        }
        return res;
    }

    string detectMPEG(File *src, int skip) {
        string res;
        unsigned char mpegheader[2];
        src->seek(skip);
        src->read((char*)mpegheader, 2);

        if (mpegheader[0] == 0xff && (mpegheader[1] & 0xe0) == 0xe0) // frame synchronizer
            if((mpegheader[1] & 0x18) != 0x08) // support MPEG 1, 2 and 2.5
                if((mpegheader[1] & 0x06) != 0x00) // Layer I, II and III
                    res = "mpeg";

        return res;
    }

    string detectSuffix(string filename) {
        // A lot of mp3s dont start with a synchronization
        // so use some suffix matching as well.

       int len = filename.length();
       if (len < 4) return "";
       string end = filename.substr(len-4, 4);

       if (end == ".mp3") return "mpeg";
       if (end == ".ogg") return "xiph";
       if (end == ".wma") return "ffmpeg";
       if (end == ".m4a") return "ffmpeg";
       if (end == ".aac") return "ffmpeg";
       if (end == ".ac3") return "ffmpeg";
       return "";
    }

    string detectFile(File *src) {
        string res;
        if (!src->openRO())
            return res;

        int skip = detectID3v2(src);
        char magic[4];
        src->seek(skip);
        src->read(magic, 4);

        if (memcmp(magic, "fLaC", 4) == 0)
            res = "xiph";
        else
        if (memcmp(magic, "OggS", 4) == 0)
            res = "xiph";
        else
        if (memcmp(magic, "MP+", 3) == 0)
            res = "mpc";
        else
        if (memcmp(magic, "\x30\x26\xb2\x75", 4) == 0) // ASF
            res = "ffmpeg";
        else
        if (memcmp(magic, ".RMF", 4) == 0)  // RealAudio
            res = "ffmpeg";
        else
        if (memcmp(magic, ".ra", 3) == 0) // Old RealAudio
            res = "ffmpeg";
        else
        if (memcmp(magic, "RIFF", 4) == 0)
            res = detectRIFF(src, skip);
        else
            res = detectMPEG(src, skip);

        if (res.empty()) res = detectSuffix(src->filename);

        src->close();
        return res;
    }

/*
    Format *detectStream(Stream *src) {
        Format *res = 0;
        if (!src->openRO())
            return 0;

        // Search for Ogg or MPEG synchronization header
        char *page = new char[4096];
        src->read(page, 4096);
        for (int i=0; i<4096; i++) {
            if (page[i] == 'O' && page[i+1] == 'g' && page[i+1] == 'g' && page[i+1] == 'S') {
                res = detectOgg(src, i);
                if (res) break;
            }
            if (page[i] == 0xff && (page[i+1] & 0x80) == 0x80) {
                res = detectMPEG(src, i);
                if (res) break;
            }
        }
        return res;
    } */
    } // namespace

} // namespace
