INCLUDES = -I$(top_srcdir)/akode/lib -I$(top_builddir)/akode/lib $(JACK_CFLAGS) $(all_includes)

lib_LTLIBRARIES	= libakode_jack_sink.la

libakode_jack_sink_la_SOURCES = jack_sink.cpp
libakode_jack_sink_la_LDFLAGS = -module -avoid-version -no-undefined $(JACK_LDFLAGS)
libakode_jack_sink_la_LIBADD = ../../lib/libakode.la $(JACK_LIBADD)
