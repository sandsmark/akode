INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/akode/lib  )


########### next target ###############

SET(akode_ffmpeg_decoder_LIB_SRCS
ffmpeg_decoder.cpp
)

KDE3_AUTOMOC(${akode_ffmpeg_decoder_LIB_SRCS})

ADD_LIBRARY(akode_ffmpeg_decoder SHARED ${akode_ffmpeg_decoder_LIB_SRCS})

TARGET_LINK_LIBRARIES(akode_ffmpeg_decoder  ${QT_AND_KDECORE_LIBS} akode )

SET_TARGET_PROPERTIES(akode_ffmpeg_decoder PROPERTIES VERSION 4.2.0 SOVERSION 4 )
INSTALL(TARGETS akode_ffmpeg_decoder DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/akode/lib -I$(top_builddir)/akode/lib $(AVCODEC_CFLAGS) $(AVFORMAT_CFLAGS) $(all_includes)
#
#lib_LTLIBRARIES	= libakode_ffmpeg_decoder.la
#
#libakode_ffmpeg_decoder_la_SOURCES = ffmpeg_decoder.cpp
#
#libakode_ffmpeg_decoder_la_LDFLAGS = -avoid-version -no-undefined $(AVCODEC_LDFLAGS) $(AVFORMAT_LDFLAGS)
#libakode_ffmpeg_decoder_la_LIBADD = ../../lib/libakode.la $(AVCODEC_LIBADD) $(AVFORMAT_LIBADD)
