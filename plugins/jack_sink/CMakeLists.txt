INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/akode/lib  )


########### next target ###############

SET(akode_jack_sink_LIB_SRCS
jack_sink.cpp
)

KDE3_AUTOMOC(${akode_jack_sink_LIB_SRCS})

ADD_LIBRARY(akode_jack_sink SHARED ${akode_jack_sink_LIB_SRCS})

TARGET_LINK_LIBRARIES(akode_jack_sink  ${QT_AND_KDECORE_LIBS} akode )

SET_TARGET_PROPERTIES(akode_jack_sink PROPERTIES VERSION 4.2.0 SOVERSION 4 )
INSTALL(TARGETS akode_jack_sink DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/akode/lib -I$(top_builddir)/akode/lib $(JACK_CFLAGS) $(all_includes)
#
#lib_LTLIBRARIES	= libakode_jack_sink.la
#
#libakode_jack_sink_la_SOURCES = jack_sink.cpp
#libakode_jack_sink_la_LDFLAGS = -module -avoid-version -no-undefined $(JACK_LDFLAGS)
#libakode_jack_sink_la_LIBADD = ../../lib/libakode.la $(JACK_LIBADD)
