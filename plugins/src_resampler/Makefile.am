INCLUDES = -I$(top_srcdir)/akode/lib -I$(top_builddir)/akode/lib -I$(srcdir)/mppdec $(all_includes)

lib_LTLIBRARIES	= libakode_src_resampler.la

libakode_src_resampler_la_SOURCES = src_resampler.cpp
libakode_src_resampler_la_LDFLAGS = -module -avoid-version -no-undefined
libakode_src_resampler_la_LIBADD = ../../lib/libakode.la $(LIBSAMPLERATE)
