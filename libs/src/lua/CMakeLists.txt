cmake_minimum_required(VERSION 2.8)

# ---------------- Main project properties ----------------

project(lua C)

set( SOURCES
    src/lapi.c
    src/lauxlib.c
    src/lbaselib.c
    src/lcode.c
    src/ldblib.c
    src/ldebug.c
    src/ldo.c
    src/ldump.c
    src/lfunc.c
    src/lgc.c
    src/linit.c
    src/liolib.c
    src/llex.c
    src/lmathlib.c
    src/lmem.c
    src/loadlib.c
    src/lobject.c
    src/lopcodes.c
    src/loslib.c
    src/lparser.c
    src/lstate.c
    src/lstring.c
    src/lstrlib.c
    src/ltable.c
    src/ltablib.c
    src/ltm.c
    src/lundump.c
    src/lvm.c
    src/lzio.c
    src/print.c 
)

# ---------------- Includes ----------------

include_directories(${CMAKE_SOURCE_DIR}/src/)

# ---------------- Targets ----------------

set( CMAKE_DEBUG_POSTFIX "d" )

add_library( "lua51" STATIC ${SOURCES} )

