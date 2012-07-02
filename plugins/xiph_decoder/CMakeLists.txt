INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib  ${CMAKE_BINARY_DIR}/lib )


########### next target ###############

SET(akode_xiph_decoder_LIB_SRCS
flac_decoder.cpp
speex_decoder.cpp
vorbis_decoder.cpp
xiph_decoder.cpp
)

ADD_LIBRARY(akode_xiph_decoder SHARED ${akode_xiph_decoder_LIB_SRCS})

TARGET_LINK_LIBRARIES(akode_xiph_decoder  ${QT_AND_KDECORE_LIBS} akode )

SET_TARGET_PROPERTIES(akode_xiph_decoder PROPERTIES VERSION 4.2.0 SOVERSION 4 )
INSTALL(TARGETS akode_xiph_decoder DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/akode/lib -I$(top_builddir)/akode/lib $(all_includes)
#
#lib_LTLIBRARIES	= libakode_xiph_decoder.la
#
#libakode_xiph_decoder_la_SOURCES = flac_decoder.cpp speex_decoder.cpp vorbis_decoder.cpp xiph_decoder.cpp
#
#libakode_xiph_decoder_la_LDFLAGS = -module -avoid-version -no-undefined
#libakode_xiph_decoder_la_LIBADD = ../../lib/libakode.la $(LIBFLAC) $(LIBOGGFLAC) $(VORBIS_LIBS) $(VORBISFILE_LIBS) $(SPEEXLIBS)
