INCLUDES = -I$(top_srcdir)/akode/lib -I$(top_builddir)/akode/lib $(all_includes)

lib_LTLIBRARIES	= libakode_sun_sink.la

libakode_sun_sink_la_SOURCES = sun_sink.cpp
libakode_sun_sink_la_LDFLAGS = -module -avoid-version -no-undefined
libakode_sun_sink_la_LIBADD = ../../lib/libakode.la $(LIBSUNAUDIO)
