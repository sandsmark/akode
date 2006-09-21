INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/akode/lib  )


########### next target ###############

SET(akode_sun_sink_LIB_SRCS
sun_sink.cpp
)

KDE3_AUTOMOC(${akode_sun_sink_LIB_SRCS})

ADD_LIBRARY(akode_sun_sink SHARED ${akode_sun_sink_LIB_SRCS})

TARGET_LINK_LIBRARIES(akode_sun_sink  ${QT_AND_KDECORE_LIBS} akode )

SET_TARGET_PROPERTIES(akode_sun_sink PROPERTIES VERSION 4.2.0 SOVERSION 4 )
INSTALL(TARGETS akode_sun_sink DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/akode/lib -I$(top_builddir)/akode/lib $(all_includes)
#
#lib_LTLIBRARIES	= libakode_sun_sink.la
#
#libakode_sun_sink_la_SOURCES = sun_sink.cpp
#libakode_sun_sink_la_LDFLAGS = -module -avoid-version -no-undefined
#libakode_sun_sink_la_LIBADD = ../../lib/libakode.la $(LIBSUNAUDIO)
