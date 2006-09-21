INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/akode/lib  )


########### next target ###############

SET(akode_oss_sink_LIB_SRCS
oss_sink.cpp
)

KDE3_AUTOMOC(${akode_oss_sink_LIB_SRCS})

ADD_LIBRARY(akode_oss_sink SHARED ${akode_oss_sink_LIB_SRCS})

TARGET_LINK_LIBRARIES(akode_oss_sink  ${QT_AND_KDECORE_LIBS} akode )

SET_TARGET_PROPERTIES(akode_oss_sink PROPERTIES VERSION 4.2.0 SOVERSION 4 )
INSTALL(TARGETS akode_oss_sink DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/akode/lib -I$(top_builddir)/akode/lib $(all_includes)
#
#lib_LTLIBRARIES	= libakode_oss_sink.la
#
#libakode_oss_sink_la_SOURCES = oss_sink.cpp
#libakode_oss_sink_la_LDFLAGS = -module -avoid-version -no-undefined
#libakode_oss_sink_la_LIBADD = ../../lib/libakode.la $(LIBOSSAUDIO)
