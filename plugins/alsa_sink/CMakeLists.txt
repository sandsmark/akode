########### next target ###############

SET(akode_alsa_sink_LIB_SRCS
alsa_sink.cpp
)


ADD_LIBRARY(akode_alsa_sink SHARED ${akode_alsa_sink_LIB_SRCS})

TARGET_LINK_LIBRARIES(akode_alsa_sink  akode )

#SET_TARGET_PROPERTIES(akode_alsa_sink PROPERTIES VERSION 4.2.0 SOVERSION 4 )
INSTALL(TARGETS akode_alsa_sink DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/akode/lib -I$(top_builddir)/akode/lib $(all_includes)
#
#lib_LTLIBRARIES	= libakode_alsa_sink.la
#
#libakode_alsa_sink_la_SOURCES = alsa_sink.cpp
#libakode_alsa_sink_la_LDFLAGS = -module -avoid-version -no-undefined
#libakode_alsa_sink_la_LIBADD = ../../lib/libakode.la $(LIBASOUND)
