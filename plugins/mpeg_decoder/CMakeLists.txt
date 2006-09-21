INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/akode/lib  )


########### next target ###############

SET(akode_mpeg_decoder_LIB_SRCS
mpeg_decoder.cpp
)

KDE3_AUTOMOC(${akode_mpeg_decoder_LIB_SRCS})

ADD_LIBRARY(akode_mpeg_decoder SHARED ${akode_mpeg_decoder_LIB_SRCS})

TARGET_LINK_LIBRARIES(akode_mpeg_decoder  ${QT_AND_KDECORE_LIBS} akode )

SET_TARGET_PROPERTIES(akode_mpeg_decoder PROPERTIES VERSION 4.2.0 SOVERSION 4 )
INSTALL(TARGETS akode_mpeg_decoder DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/akode/lib -I$(top_builddir)/akode/lib $(all_includes)
#
#lib_LTLIBRARIES	= libakode_mpeg_decoder.la
#
#libakode_mpeg_decoder_la_SOURCES = mpeg_decoder.cpp
#
#libakode_mpeg_decoder_la_LDFLAGS = -avoid-version -no-undefined
#libakode_mpeg_decoder_la_LIBADD = ../../lib/libakode.la $(MADLIBS)
