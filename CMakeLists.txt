project(akode)

set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)" )
set(EXEC_INSTALL_PREFIX  ${CMAKE_INSTALL_PREFIX}       CACHE PATH  "Base directory for executables and libraries" FORCE)
#
## the following are directories where stuff will be installed to
set(BIN_INSTALL_DIR          "${EXEC_INSTALL_PREFIX}/bin"                  CACHE PATH "The kde info install dir (default prefix/info)" FORCE)
set(LIB_INSTALL_DIR          "${EXEC_INSTALL_PREFIX}/lib${LIB_SUFFIX}"     CACHE PATH "The subdirectory relative to the install prefix where libraries will be installed (default is /lib${LIB_SUFFIX})" FORCE)
set(INCLUDE_INSTALL_DIR      "${CMAKE_INSTALL_PREFIX}/include"             CACHE PATH "The subdirectory to the header prefix" FORCE)


include_directories(${CMAKE_BINARY_DIR}/)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/akode-config.cmake ${CMAKE_CURRENT_BINARY_DIR}/akode-config )

add_subdirectory( lib ) 
add_subdirectory( plugins ) 
add_subdirectory( akodeplay ) 


########### install files ###############

install( FILES  akode-config DESTINATION ${BIN_INSTALL_DIR})
