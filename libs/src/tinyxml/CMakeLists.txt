cmake_minimum_required(VERSION 2.8)

# ---------------- Main project properties ----------------

project(tinyxml CXX)

set( SOURCES
    tinystr.cpp
    tinyxml.cpp
    tinyxmlerror.cpp
    tinyxmlparser.cpp
)

# ---------------- Includes ----------------

include_directories(${CMAKE_SOURCE_DIR})

# ---------------- Targets ----------------

set( CMAKE_DEBUG_POSTFIX "d" )

add_library( "tinyxml" STATIC ${SOURCES} )
add_definitions( -DTIXML_USE_STL )
