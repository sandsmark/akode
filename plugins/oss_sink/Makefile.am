INCLUDES = -I$(top_srcdir)/akode/lib -I$(top_builddir)/akode/lib $(all_includes)

lib_LTLIBRARIES	= libakode_oss_sink.la

libakode_oss_sink_la_SOURCES = oss_sink.cpp
libakode_oss_sink_la_LDFLAGS = -module -avoid-version -no-undefined
libakode_oss_sink_la_LIBADD = ../../lib/libakode.la $(LIBOSSAUDIO)
