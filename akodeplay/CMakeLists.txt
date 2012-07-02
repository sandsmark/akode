INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/akode/lib ${CMAKE_BINARY_DIR}/lib )


########### next target ###############

SET(akodeplay_SRCS
akodeplay.cpp
)

ADD_EXECUTABLE(akodeplay ${akodeplay_SRCS})

TARGET_LINK_LIBRARIES(akodeplay akode pthread dl )

INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/akodeplay DESTINATION ${BIN_INSTALL_DIR})
