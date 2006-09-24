if(HAVE_SYS_AUDIOIO_H)
	ADD_SUBDIRECTORY(sun_sink)
endif(HAVE_SYS_AUDIOIO_H)
MESSAGE(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(AKODE_MPC_DECODER)")
MESSAGE(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(AKODE_XIPH_DECODER)")
MESSAGE(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(AKODE_MPEG_DECODER)")
MESSAGE(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(AKODE_FFMPEG_DECODER)")
MESSAGE(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(AKODE_ALSA_SINK)")
MESSAGE(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(AKODE_OSS_SINK)")
MESSAGE(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(AKODE_JACK_SINK)")
MESSAGE(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(AKODE_POLYP_SINK)")
MESSAGE(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(AKODE_SRC_RESAMPLER)")

########### install files ###############




#original Makefile.am contents follow:

#AKODE_MPC_DECODER=mpc_decoder
#
#if include_xiph_decoder
#AKODE_XIPH_DECODER=xiph_decoder
#endif
#
#if include_mpeg_decoder
#AKODE_MPEG_DECODER=mpeg_decoder
#endif
#
#if include_ffmpeg_decoder
#AKODE_FFMPEG_DECODER=ffmpeg_decoder
#endif
#
#if include_src_resampler
#AKODE_SRC_RESAMPLER=src_resampler
#endif
#
#if include_alsa_sink
#AKODE_ALSA_SINK=alsa_sink
#endif
#
#if include_oss_sink
#AKODE_OSS_SINK=oss_sink
#endif
#if include_jack_sink
#AKODE_JACK_SINK=jack_sink
#endif
#
#if include_polyp_sink
#AKODE_POLYP_SINK=polyp_sink
