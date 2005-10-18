INCLUDES = -I$(top_srcdir)/akode/lib -I$(top_builddir)/akode/lib -I/opt/ffmpeg/include $(all_includes)

lib_LTLIBRARIES	= libakode_ffmpeg_decoder.la

libakode_ffmpeg_decoder_la_SOURCES = ffmpeg_decoder.cpp

libakode_ffmpeg_decoder_la_LDFLAGS = -avoid-version -no-undefined
libakode_ffmpeg_decoder_la_LIBADD = ../../lib/libakode.la -L/opt/ffmpeg/lib -R/opt/ffmpeg/lib -lavformat-CVS -lavcodec-CVS

mcopclassdir = $(libdir)/mcop
mcopclass_DATA = akodeFFMPEGPlayObject.mcopclass
