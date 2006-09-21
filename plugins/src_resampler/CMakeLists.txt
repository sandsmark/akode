INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/akode/lib  )


########### next target ###############

SET(akode_src_resampler_LIB_SRCS
src_resampler.cpp
)

KDE3_AUTOMOC(${akode_src_resampler_LIB_SRCS})

ADD_LIBRARY(akode_src_resampler SHARED ${akode_src_resampler_LIB_SRCS})

TARGET_LINK_LIBRARIES(akode_src_resampler  ${QT_AND_KDECORE_LIBS} akode )

SET_TARGET_PROPERTIES(akode_src_resampler PROPERTIES VERSION 4.2.0 SOVERSION 4 )
INSTALL(TARGETS akode_src_resampler DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/akode/lib -I$(top_builddir)/akode/lib -I$(srcdir)/mppdec $(all_includes)
#
#lib_LTLIBRARIES	= libakode_src_resampler.la
#
#libakode_src_resampler_la_SOURCES = src_resampler.cpp
#libakode_src_resampler_la_LDFLAGS = -module -avoid-version -no-undefined
#libakode_src_resampler_la_LIBADD = ../../lib/libakode.la $(LIBSAMPLERATE)
