INCLUDES = -I$(top_srcdir)/akode/lib -I$(top_builddir)/akode/lib $(all_includes)

lib_LTLIBRARIES	= libakode_alsa_sink.la

libakode_alsa_sink_la_SOURCES = alsa_sink.cpp
libakode_alsa_sink_la_LDFLAGS = -module -avoid-version -no-undefined
libakode_alsa_sink_la_LIBADD = ../../lib/libakode.la $(LIBASOUND)
