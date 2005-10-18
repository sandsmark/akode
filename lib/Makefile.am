INCLUDES = $(all_includes)

lib_LTLIBRARIES	= libakode.la

libakode_la_SOURCES = bytebuffer.cpp audiobuffer.cpp pluginhandler.cpp \
                      decoderpluginhandler.cpp resamplerpluginhandler.cpp \
                      sinkpluginhandler.cpp encoderpluginhandler.cpp \
		      fast_resampler.cpp crossfader.cpp volumefilter.cpp \
                      localfile.cpp mmapfile.cpp \
                      wav_decoder.cpp auto_sink.cpp void_sink.cpp \
                      converter.cpp buffered_decoder.cpp \
                      player.cpp magic.cpp

AM_CPPFLAGS = -DAKODE_SEARCHDIR=\"$(libdir)\"

libakode_la_LDFLAGS = -no-undefined -version-info 2:0:0 $(all_libraries)
libakode_la_LIBADD =  $(LIBPTHREAD) $(AKODE_LIBDL)

libakode_includedir	= $(includedir)/akode
libakode_include_HEADERS = akode_export.h akodelib.h decoder.h sink.h encoder.h \
	audioconfiguration.h audioframe.h audiobuffer.h bytebuffer.h \
	file.h localfile.h mmapfile.h pluginhandler.h \
	crossfader.h volumefilter.h resampler.h fast_resampler.h \
	buffered_decoder.h wav_decoder.h auto_sink.h void_sink.h \
	player.h magic.h converter.h framedecoder.h
