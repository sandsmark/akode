INCLUDES = -I$(top_srcdir)/akode/lib -I$(top_builddir)/akode/lib $(AVCODEC_CFLAGS) $(AVFORMAT_CFLAGS) $(all_includes)

lib_LTLIBRARIES	= libakode_ffmpeg_decoder.la

libakode_ffmpeg_decoder_la_SOURCES = ffmpeg_decoder.cpp

libakode_ffmpeg_decoder_la_LDFLAGS = -avoid-version -no-undefined $(AVCODEC_LDFLAGS) $(AVFORMAT_LDFLAGS)
libakode_ffmpeg_decoder_la_LIBADD = ../../lib/libakode.la $(AVCODEC_LIBADD) $(AVFORMAT_LIBADD)

mcopclassdir = $(libdir)/mcop
mcopclass_DATA = akodeFFMPEGPlayObject.mcopclass
