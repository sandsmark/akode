INCLUDES = -I$(top_srcdir)/akode/lib -I$(top_builddir)/akode/lib $(POLYP_CFLAGS) $(all_includes)

lib_LTLIBRARIES	= libakode_polyp_sink.la

libakode_polyp_sink_la_SOURCES = polyp_sink.cpp
libakode_polyp_sink_la_LDFLAGS = -module -avoid-version -no-undefined $(POLYP_LDFLAGS)
libakode_polyp_sink_la_LIBADD = ../../lib/libakode.la $(POLYP_LIBADD)
