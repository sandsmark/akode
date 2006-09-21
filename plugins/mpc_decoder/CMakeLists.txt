ADD_SUBDIRECTORY( mppdec ) 

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/akode/lib  )


########### next target ###############

SET(akode_mpc_decoder_LIB_SRCS
mpc_decoder.cpp
)

KDE3_AUTOMOC(${akode_mpc_decoder_LIB_SRCS})

ADD_LIBRARY(akode_mpc_decoder SHARED ${akode_mpc_decoder_LIB_SRCS})

TARGET_LINK_LIBRARIES(akode_mpc_decoder  ${QT_AND_KDECORE_LIBS} akode akode_mppdec )

SET_TARGET_PROPERTIES(akode_mpc_decoder PROPERTIES VERSION 4.2.0 SOVERSION 4 )
INSTALL(TARGETS akode_mpc_decoder DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############




#original Makefile.am contents follow:

#SUBDIRS = mppdec .
#INCLUDES = -I$(top_srcdir)/akode/lib -I$(top_builddir)/akode/lib -I$(srcdir)/mppdec $(all_includes)
#
#lib_LTLIBRARIES	= libakode_mpc_decoder.la
#
#libakode_mpc_decoder_la_SOURCES = mpc_decoder.cpp
#libakode_mpc_decoder_la_LDFLAGS = -module -avoid-version -no-undefined
#libakode_mpc_decoder_la_LIBADD = ../../lib/libakode.la mppdec/libakode_mppdec.la
