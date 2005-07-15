SUBDIRS = mppdec .
INCLUDES = -I$(top_srcdir)/akode/lib -I$(top_builddir)/akode/lib -I$(srcdir)/mppdec $(all_includes)

lib_LTLIBRARIES	= libakode_mpc_decoder.la

libakode_mpc_decoder_la_SOURCES = mpc_decoder.cpp
libakode_mpc_decoder_la_LDFLAGS = -module -avoid-version -no-undefined
libakode_mpc_decoder_la_LIBADD = ../../lib/libakode.la mppdec/libakode_mppdec.la

mcopclassdir = $(libdir)/mcop
mcopclass_DATA = akodeMPCPlayObject.mcopclass
