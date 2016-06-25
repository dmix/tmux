cmake_minimum_required(VERSION 2.8.12)
project(tmux)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${tmux_SOURCE_DIR}/cmake")
set(BIN_DIR ${tmux_SOURCE_DIR}/bin)

# Bump up warning levels tmuxropriately for clang, gcc & msvc and build in debug mode
if (${CMAKE_C_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -g")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O2")
elseif (${CMAKE_C_COMPILER_ID} STREQUAL "MSVC")
	if (CMAKE_C_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
	else()
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
	endif()
endif()














macro(ADD_COND var cond item)
  if(${cond})
    set(${var} ${${var}} ${item})
  endif()
endmacro()


include(CheckSymbolExists)
include(CheckIncludeFile)
include(CheckLibraryExists)
include(CheckFunctionExists)


set(CFG_HEADERS )
set(tmux_COMPAT )


check_include_file(string.h HAVE_STRING_H)
add_cond(CFG_HEADERS HAVE_STRING_H string.h)

check_include_file(sys/queue.h HAVE_QUEUE_H)
check_include_file(sys/tree.h HAVE_TREE_H)
check_include_file(bitstring.h HAVE_BITSTRING_H)
check_include_file(path.h HAVE_PATH_H)
check_include_file(fcntl.h HAVE_FCNTL_H)
add_cond(CFG_HEADERS HAVE_FCNTL_H fcntl.h)

set(LIBUTIL_LIBRARIES )
check_function_exists("openpty" HAVE_BUILTIN_OPENPTY)
if(HAVE_BUILTIN_OPENPTY)
  # Libutil functions are builtin the environment (e.g emscripten)
  set(LIBUTIL_LIBRARIES )
  set(HAVE_LIBUTIL 1)
else()
  find_library(HAVE_LIBUTIL util)
  set(LIBUTIL_LIBRARIES ${HAVE_LIBUTIL})
endif()
add_cond(CMAKE_REQUIRED_LIBRARIES HAVE_LIBUTIL "${LIBUTIL_LIBRARIES}")



list(APPEND CMAKE_REQUIRED_LIBRARIES util)
CHECK_FUNCTION_EXISTS(forkpty HAS_FORKPTY)
check_include_file(pty.h HAVE_PTY_H)
add_cond(CFG_HEADERS HAVE_PTY_H pty.h)

if(HAVE_QUEUE_H)
	list(APPEND CMAKE_REQUIRED_DEFINITIONS -DQUEUE_H)
endif()

# add_cond(CFG_HEADERS HAVE_STDLIB_H stdlib.h)
# add_cond(CFG_HEADERS HAVE_QUEUE_H queue.h)
# add_cond(CFG_HEADERS HAVE_TREE_H sys/tree.h)
# add_cond(CFG_HEADERS HAVE_BITSTRING_H bitstring.h)
# add_cond(CFG_HEADERS HAVE_PATHS_H paths.h)


#CHECK_FUNCTION_EXISTS(getopt HAVE_GETOPT)
if (NOT HAVE_GETOPT)
	set(tmux_COMPAT ${tmux_COMPAT} compat/getopt.c)
	set(NO_GETOPT 1)
	add_definitions(-DNO_GETOPT=1)
endif()

check_symbol_exists(strlcat      "${CFG_HEADERS}" HAVE_STRLCAT)
if (HAVE_STRLCAT)
  add_definitions(-DHAVE_STRLCAT=1)
endif ()
check_symbol_exists(strlcpy      "${CFG_HEADERS}" HAVE_STRLCPY)

if (HAVE_STRLCPY)
  add_definitions(-DHAVE_STRLCPY=1)
endif ()

check_symbol_exists(openat       "${CFG_HEADERS}" HAVE_OPENAT)
if(HAVE_OPENAT)
	list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_OPENAT)
endif()


check_symbol_exists(flock        "${CFG_HEADERS}" HAVE_FLOCK)
if(NOT HAVE_FLOCK)
  check_library_exists(bsd flock "" FLOCK_NEEDS_LIBBSD)
endif()


set(CMAKE_EXTRA_INCLUDE_FILES ${CFG_HEADERS})

message(STATUS "compat files: ${tmux_COMPAT}")












find_package(Curses REQUIRED)
find_package(Libevent REQUIRED)
include_directories(${CURSES_INCLUDE_DIR} ${LIBEVENT_INCLUDE_DIRS})

include_directories("./")
file (GLOB tmux_SOURCES tmux.c osdep-darwin.c forkpty-darwin.c client.c)

file (STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/configure.ac" CONFIGURE_AC REGEX "AC_INIT\\(.*\\)" )
string(REGEX REPLACE "AC_INIT\\(.*, ([0-9]+\\.[0-9]+(-dev)?)\\)" "\\1" PACKAGE_VERSION ${CONFIGURE_AC})

message(STATUS "Parsed tmux package version: ${PACKAGE_VERSION}")

# These are internal to CMake
string(REGEX REPLACE "([0-9]+\\.[0-9]+)(-dev)?" "\\1" tmux_VERSION ${PACKAGE_VERSION})
string(REGEX REPLACE "([0-9]+)\\.[0-9]+" "\\1" tmux_VERSION_MAJOR ${tmux_VERSION})
string(REGEX REPLACE "[0-9]+\\.([0-9])+" "\\1" tmux_VERSION_MINOR ${tmux_VERSION})
message(STATUS "Parsed tmux version: ${tmux_VERSION} (${tmux_VERSION_MAJOR}.${tmux_VERSION_MINOR})")

add_definitions(-DVERSION="${tmux_VERSION}")





add_executable(tmux ${tmux_COMPAT} ${tmux_SOURCES})
target_link_libraries(tmux ${CURSES_LIBRARY} ${LIBEVENT_LIBRARIES})
install(TARGETS tmux RUNTIME DESTINATION ${BIN_DIR})
