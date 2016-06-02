cmake_minimum_required(VERSION 2.6)
project(tmux)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${tmux_SOURCE_DIR}/cmake")
set(BIN_DIR ${tmux_SOURCE_DIR}/bin)

# Bump up warning levels tmuxropriately for clang, gcc & msvc and build in debug mode
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -std=c99")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O2")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
	if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
endif()

find_package(Curses REQUIRED)
find_package(Libevent REQUIRED)
include_directories(${CURSES_INCLUDE_DIR} ${LIBEVENT_INCLUDE_DIRS})

include_directories("./")
file (GLOB tmux_SOURCES tmux.c osdep-darwin.c forkpty-darwin.c )

file (STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/configure.ac" CONFIGURE_AC REGEX "AC_INIT\\(.*\\)" )
string(REGEX REPLACE "AC_INIT\\(\\[.*\\], \\[([0-9]+\\.[0-9]+\\.[0-9]+(-dev)?)\\]\\)" "\\1" PACKAGE_VERSION ${CONFIGURE_AC})

message(STATUS "Parsed tmux package version: ${PACKAGE_VERSION}")

# These are internal to CMake
string(REGEX REPLACE "([0-9]+\\.[0-9]+\\.[0-9]+)(-dev)?" "\\1" tmux_VERSION ${PACKAGE_VERSION})
string(REGEX REPLACE "([0-9]+)\\.[0-9]+\\.[0-9]+" "\\1" tmux_VERSION_MAJOR ${tmux_VERSION})
string(REGEX REPLACE "[0-9]+\\.([0-9])+\\.[0-9]+" "\\1" tmux_VERSION_MINOR ${tmux_VERSION})
string(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+)" "\\1" tmux_VERSION_PATCH ${tmux_VERSION})
message(STATUS "Parsed tmux version: ${tmux_VERSION} (${tmux_VERSION_MAJOR}.${tmux_VERSION_MINOR}.${tmux_VERSION_PATCH})")


add_executable(tmux ${tmux_SOURCES})
target_link_libraries(tmux ${CURSES_LIBRARY} ${LIBEVENT_LIBRARIES})
install(TARGETS tmux RUNTIME DESTINATION ${BIN_DIR})
