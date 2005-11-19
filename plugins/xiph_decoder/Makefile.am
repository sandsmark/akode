INCLUDES = -I$(top_srcdir)/akode/lib -I$(top_builddir)/akode/lib $(all_includes)

lib_LTLIBRARIES	= libakode_xiph_decoder.la

libakode_xiph_decoder_la_SOURCES = flac_decoder.cpp speex_decoder.cpp vorbis_decoder.cpp xiph_decoder.cpp

libakode_xiph_decoder_la_LDFLAGS = -module -avoid-version -no-undefined
libakode_xiph_decoder_la_LIBADD = ../../lib/libakode.la $(LIBFLAC) $(LIBOGGFLAC) $(VORBIS_LIBS) $(VORBISFILE_LIBS) $(SPEEXLIBS)