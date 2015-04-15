cmake_minimum_required(VERSION 2.8)

# ---------------- Main project properties ----------------

project(luabind CXX)

set( SOURCES
    src/class.cpp
    src/class_info.cpp
    src/class_registry.cpp
    src/class_rep.cpp
    src/create_class.cpp
    src/error.cpp
    src/exception_handler.cpp
    src/function.cpp
    src/inheritance.cpp
    src/link_compatibility.cpp
    src/object_rep.cpp
    src/open.cpp
    src/pcall.cpp
    src/scope.cpp
    src/stack_content_by_name.cpp
    src/weak_ref.cpp
    src/wrapper_base.cpp 
)

# ---------------- Includes ----------------

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/../lua/src/)
include_directories(${CMAKE_SOURCE_DIR}/../../../src/)

# ---------------- Targets ----------------

set( CMAKE_DEBUG_POSTFIX "d" )

#add_definitions( -DLUABIND_CPP0x )

add_library( "luabind" STATIC ${SOURCES} )

