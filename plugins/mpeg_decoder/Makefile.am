INCLUDES = -I$(top_srcdir)/akode/lib -I$(top_builddir)/akode/lib $(all_includes)

lib_LTLIBRARIES	= libakode_mpeg_decoder.la

libakode_mpeg_decoder_la_SOURCES = mpeg_decoder.cpp

libakode_mpeg_decoder_la_LDFLAGS = -avoid-version -no-undefined
libakode_mpeg_decoder_la_LIBADD = ../../lib/libakode.la $(MADLIBS)

mcopclassdir = $(libdir)/mcop
mcopclass_DATA = akodeMPEGPlayObject.mcopclass
