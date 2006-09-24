# NOTE: only add something here if it is really needed by all of kdelibs.
#     Otherwise please prefer adding to the relevant config-foo.h.cmake file,
#     and the CMakeLists.txt that generates it (or a separate ConfigureChecks.make file if you prefer)
#     to minimize recompilations and increase modularity.

include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckTypeSize)
include(CheckCXXSourceCompiles)

check_include_files(sys/audioio.h HAVE_SYS_AUDIOIO_H)

