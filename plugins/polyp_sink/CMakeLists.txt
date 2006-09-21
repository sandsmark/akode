INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/akode/lib  )


########### next target ###############

SET(akode_polyp_sink_LIB_SRCS
polyp_sink.cpp
)

KDE3_AUTOMOC(${akode_polyp_sink_LIB_SRCS})

ADD_LIBRARY(akode_polyp_sink SHARED ${akode_polyp_sink_LIB_SRCS})

TARGET_LINK_LIBRARIES(akode_polyp_sink  ${QT_AND_KDECORE_LIBS} akode )

SET_TARGET_PROPERTIES(akode_polyp_sink PROPERTIES VERSION 4.2.0 SOVERSION 4 )
INSTALL(TARGETS akode_polyp_sink DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/akode/lib -I$(top_builddir)/akode/lib $(POLYP_CFLAGS) $(all_includes)
#
#lib_LTLIBRARIES	= libakode_polyp_sink.la
#
#libakode_polyp_sink_la_SOURCES = polyp_sink.cpp
#libakode_polyp_sink_la_LDFLAGS = -module -avoid-version -no-undefined $(POLYP_LDFLAGS)
#libakode_polyp_sink_la_LIBADD = ../../lib/libakode.la $(POLYP_LIBADD)
