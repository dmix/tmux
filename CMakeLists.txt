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
endif()

check_symbol_exists(strlcat      "${CFG_HEADERS}" HAVE_STRLCAT)
if (HAVE_STRLCAT)
  add_definitions(-DHAVE_STRLCAT=1)
endif ()
check_symbol_exists(strlcpy      "${CFG_HEADERS}" HAVE_STRLCPY)

if (HAVE_STRLCPY)
  add_definitions(-DHAVE_STRLCPY=1)
endif ()


check_library_exists(util imsg_init "" HAVE_IMSG)
if(HAVE_IMSG)
	add_definitions(-DHAVE_IMSG=1)
else()
	set(tmux_COMPAT ${tmux_COMPAT} compat/imsg.c compat/imsg-buffer.c)
endif()



check_function_exists(b64_ntop HAVE_B64_NTOP)
if(HAVE_B64_NTOP)
	add_definitions(-DHAVE_B64_NTOP)
else()
	set(tmux_COMPAT ${tmux_COMPAT} compat/b64_ntop.c)
endif()


check_function_exists(closefrom HAVE_CLOSEFROM)
if(HAVE_CLOSEFROM)
	add_definitions(-DHAVE_CLOSEFROM)
else()
	set(tmux_COMPAT ${tmux_COMPAT} compat/closefrom.c)
endif()


check_function_exists(reallocarray HAVE_REALLOCARRAY)
if(HAVE_REALLOCARRAY)
	add_definitions(-DHAVE_REALLOCARRAY)
else()
	set(tmux_COMPAT ${tmux_COMPAT} compat/reallocarray.c)
endif()


check_function_exists(setproctitle HAVE_SETPROCTITLE)
if(HAVE_SETPROCTITLE)
	add_definitions(-DHAVE_SETPROCTITLE)
else()
	set(tmux_COMPAT ${tmux_COMPAT} compat/setproctitle.c)
endif()


# Look for stravis, compat/{vis,unvis}.c used if missing.
check_function_exists(stravis HAVE_STRAVIS)
if(HAVE_STRAVIS)
	add_definitions(-DHAVE_VIS)
else()
	set(tmux_COMPAT ${tmux_COMPAT} compat/vis.c)
endif()


# Look for strtonum, compat/strtonum.c used if missing.
check_function_exists(strtonum HAVE_STRTONUM)
if(HAVE_STRTONUM)
	add_definitions(-DHAVE_STRTONUM)
else()
	set(tmux_COMPAT ${tmux_COMPAT} compat/strtonum.c)
endif()



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

if (CURSES_HAVE_CURSES_H)
	add_definitions(-DHAVE_CURSES_H=1)
endif()
if (CURSES_HAVE_NCURSES_H)
	add_definitions(-DHAVE_NCURSES_H=1)
endif()
find_package(Libevent REQUIRED)
include_directories(${CURSES_INCLUDE_DIR} ${LIBEVENT_INCLUDE_DIRS})

include_directories("./")
# file (GLOB tmux_SOURCES tmux.c osdep-darwin.c forkpty-darwin.c client.c)

file (GLOB tmux_SOURCES
	alerts.c
	arguments.c
	attributes.c
	cfg.c
	client.c
	cmd-attach-session.c
	cmd-bind-key.c
	cmd-break-pane.c
	cmd-capture-pane.c
	cmd-choose-buffer.c
	cmd-choose-client.c
	cmd-choose-tree.c
	cmd-clear-history.c
	cmd-command-prompt.c
	cmd-confirm-before.c
	cmd-copy-mode.c
	cmd-detach-client.c
	cmd-display-message.c
	cmd-display-panes.c
	cmd-find.c
	cmd-find-window.c
	cmd-if-shell.c
	cmd-join-pane.c
	cmd-kill-pane.c
	cmd-kill-server.c
	cmd-kill-session.c
	cmd-kill-window.c
	cmd-list-buffers.c
	cmd-list-clients.c
	cmd-list-keys.c
	cmd-list-panes.c
	cmd-list-sessions.c
	cmd-list-windows.c
	cmd-list.c
	cmd-load-buffer.c
	cmd-lock-server.c
	cmd-move-window.c
	cmd-new-session.c
	cmd-new-window.c
	cmd-paste-buffer.c
	cmd-pipe-pane.c
	cmd-queue.c
	cmd-refresh-client.c
	cmd-rename-session.c
	cmd-rename-window.c
	cmd-resize-pane.c
	cmd-respawn-pane.c
	cmd-respawn-window.c
	cmd-rotate-window.c
	cmd-run-shell.c
	cmd-save-buffer.c
	cmd-select-layout.c
	cmd-select-pane.c
	cmd-select-window.c
	cmd-send-keys.c
	cmd-set-buffer.c
	cmd-set-environment.c
	cmd-set-hook.c
	cmd-set-option.c
	cmd-show-environment.c
	cmd-show-messages.c
	cmd-show-options.c
	cmd-source-file.c
	cmd-split-window.c
	cmd-string.c
	cmd-swap-pane.c
	cmd-swap-window.c
	cmd-switch-client.c
	cmd-unbind-key.c
	cmd-wait-for.c
	cmd.c
	colour.c
	control.c
	control-notify.c
	environ.c
	format.c
	grid-view.c
	grid.c
	hooks.c
	input-keys.c
	input.c
	job.c
	key-bindings.c
	key-string.c
	layout-custom.c
	layout-set.c
	layout.c
	log.c
	mode-key.c
	names.c
	notify.c
	options-table.c
	options.c
	paste.c
	proc.c
	resize.c
	screen-redraw.c
	screen-write.c
	screen.c
	server-client.c
	server-fn.c
	server.c
	session.c
	signal.c
	status.c
	style.c
	tmux.c
	tty-acs.c
	tty-keys.c
	tty-term.c
	tty.c
	utf8.c
	window-choose.c
	window-clock.c
	window-copy.c
	window.c
	xmalloc.c
	xterm-keys.c
)

# Get the platform
if(WIN32)
  if(NOT WINDOWS)
    set(WINDOWS TRUE)
  endif()
elseif(UNIX AND NOT APPLE)
  if(CMAKE_SYSTEM_NAME MATCHES ".*Linux")
    set(LINUX TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "kFreeBSD.*")
    set(FREEBSD TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "kNetBSD.*|NetBSD.*")
    set(NETBSD TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "kOpenBSD.*|OpenBSD.*")
    set(OPENBSD TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "kFreeBSD.*|FreeBSD.*")
    set(FREEBSD TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "DragonFly.*")
    set(DRAGONFLY TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "Solaris.*|SunOS.*")
    set(SOLARIS TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "HP-UX.*")
    set(HPUX TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "AIX.*")
    set(AIX TRUE)
  endif()
elseif(APPLE)
  if(CMAKE_SYSTEM_NAME MATCHES ".*Darwin.*")
    set(DARWIN TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES ".*MacOS.*")
    set(MACOSX TRUE)
  endif()
endif()

# OS-dependent C files
if (DARWIN)
	set(tmux_COMPAT ${tmux_COMPAT} osdep-darwin.c)
elseif(LINUX)
	set(tmux_COMPAT ${tmux_COMPAT} osdep-linux.c)
elseif(CYGWIN)
	set(tmux_COMPAT ${tmux_COMPAT} osdep-cygwin.c)
elseif(DRAGONFLY)
	set(tmux_COMPAT ${tmux_COMPAT} osdep-dragonfly.c)
elseif(FREEBSD)
	set(tmux_COMPAT ${tmux_COMPAT} osdep-freebsd.c)
elseif(NETBSD)
	set(tmux_COMPAT ${tmux_COMPAT} osdep-netbsd.c)
elseif(OPENBSD)
	set(tmux_COMPAT ${tmux_COMPAT} osdep-openbsd.c)
elseif(SUNOS)
	set(tmux_COMPAT ${tmux_COMPAT} osdep-sunos.c)
elseif(HPUX)
	set(tmux_COMPAT ${tmux_COMPAT} osdep-hpux.c)
elseif(AIX)
	set(tmux_COMPAT ${tmux_COMPAT} osdep-aix.c)
else()
	set(tmux_COMPAT ${tmux_COMPAT} osdep-unknown.c)
endif()




#
# GET VERSION
#

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
