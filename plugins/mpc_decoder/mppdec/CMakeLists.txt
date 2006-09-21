INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/akode/lib  )


########### next target ###############

SET(akode_mppdec_STAT_SRCS
bitstream.cpp
huffsv46.cpp
huffsv7.cpp
idtag.cpp
mpc_dec.cpp
requant.cpp
streaminfo.cpp
synth_filter.cpp
)

KDE3_AUTOMOC(${akode_mppdec_STAT_SRCS})

ADD_LIBRARY(akode_mppdec STATIC ${akode_mppdec_STAT_SRCS})


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/akode/lib $(all_includes)
#noinst_LTLIBRARIES	= libakode_mppdec.la
#
#libakode_mppdec_la_SOURCES = bitstream.cpp huffsv46.cpp huffsv7.cpp idtag.cpp mpc_dec.cpp requant.cpp streaminfo.cpp synth_filter.cpp
#
## workaround
#KDE_OPTIONS = nofinal
