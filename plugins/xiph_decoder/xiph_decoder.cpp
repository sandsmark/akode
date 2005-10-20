/*  aKode: Xiph-Format

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
    along with this library; see the src COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "akodelib.h"

#include "file.h"
#include "audioframe.h"
#include "decoder.h"

#include "xiph_decoder.h"
#include "flac_decoder.h"
#include "vorbis_decoder.h"
#include "speex_decoder.h"

namespace aKode {

bool XiphDecoderPlugin::canDecode(File* src) {
#ifdef HAVE_LIBFLAC
    if (flac_decoder.canDecode(src))
        return true;
    else
#endif
#ifdef HAVE_LIBOGGFLAC
    if (oggflac_decoder.canDecode(src))
        return true;
    else
#endif
#ifdef HAVE_OGG_VORBIS
    if (vorbis_decoder.canDecode(src))
        return true;
    else
#endif
#ifdef HAVE_SPEEX
    if (speex_decoder.canDecode(src))
        return true;
    else
#endif
        return false;
}

Decoder* XiphDecoderPlugin::openDecoder(File* src) {
#ifdef HAVE_LIBFLAC
    if (flac_decoder.canDecode(src))
        return flac_decoder.openDecoder(src);
    else
#endif
#ifdef HAVE_LIBOGGFLAC
    if (oggflac_decoder.canDecode(src))
        return oggflac_decoder.openDecoder(src);
    else
#endif
#ifdef HAVE_OGG_VORBIS
    if (vorbis_decoder.canDecode(src))
        return vorbis_decoder.openDecoder(src);
    else
#endif
#ifdef HAVE_SPEEX
    if (speex_decoder.canDecode(src))
        return speex_decoder.openDecoder(src);
    else
#endif
        return 0;
}

extern "C" { XiphDecoderPlugin xiph_decoder; }

} // namespace
