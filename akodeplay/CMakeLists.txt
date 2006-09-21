INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/akode/lib  )


########### next target ###############

SET(akodeplay_SRCS
akodeplay.cpp
)

ADD_EXECUTABLE(akodeplay ${akodeplay_SRCS})

TARGET_LINK_LIBRARIES(akodeplay akode )

INSTALL(PROGRAMS akodeplay DESTINATION ${BIN_INSTALL_DIR})
