# -----------------------------------------------------------------------------
# Author:  Garrett Smith
# File:    CMakeLists.cmake
# Created: 08/29/2011
# -----------------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8.4 FATAL_ERROR)
project(attack-framework)

# set default project options

option(BUILD_TESTS     "Build test programs and unit tests"     OFF)
option(BUILD_REFERENCE "Build DPA reference implementations"    OFF)
option(BUILD_DOXYGEN   "Build source documentation via doxygen" OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

set(DPA_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_MODULE_PATH ${DPA_MODULE_PATH} ${CMAKE_MODULE_PATH})

# detect the platform, architecture, and precompiled header support

include(${DPA_MODULE_PATH}/DetectPlatform.cmake)
include(${DPA_MODULE_PATH}/DetectArchitecture.cmake)

# encode version number as <major>.<minor>.<revision>:<changeset>

set(DPA_VER_MAJ "0")
set(DPA_VER_MIN "0")
set(DPA_VER_REV "1")

find_package(Git)
if(GIT_FOUND)
    GIT_WC_INFO(${PROJECT_SOURCE_DIR} DPA)
    set(DPA_VER_ID ${DPA_WC_REVISION})
endif(GIT_FOUND)

set(BUILD_VERSION "${DPA_VER_MAJ}.${DPA_VER_MIN}.${DPA_VER_REV}:${DPA_VER_ID}")

# perform configuration and generate config.h file

include(CheckIncludeFiles)
check_include_files(intrin.h HAVE_INTRIN_H)
check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(windows.h HAVE_WINDOWS_H)

include(CheckFunctionExists)
check_function_exists(clock_gettime HAVE_CLOCK_GETTIME)

configure_file(
    ${CMAKE_SOURCE_DIR}/src/common/config.h.cmake
    ${CMAKE_BINARY_DIR}/src/common/config.h
)

include_directories(
    "${CMAKE_SOURCE_DIR}/src/common"
)

message(STATUS "--------------------------------------------------------------")
message(STATUS "CMAKE_BUILD_TYPE:       ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_INSTALL_PREFIX:   ${CMAKE_INSTALL_PREFIX}")
message(STATUS "CMAKE_VERBOSE_MAKEFILE: ${CMAKE_VERBOSE_MAKEFILE}")
message(STATUS "BUILD_TESTS:            ${BUILD_TESTS}")          
message(STATUS "BUILD_REFERENCE:        ${BUILD_REFERENCE}")
message(STATUS "BUILD_DOXYGEN:          ${BUILD_DOXYGEN}")
message(STATUS "BUILD_VERSION:          ${BUILD_VERSION}")
message(STATUS "--------------------------------------------------------------")

set(BUILD_TESTS ${BUILD_TESTS} CACHE BOOL
    "Build test programs and unit tests" FORCE)
set(BUILD_REFERENCE ${BUILD_REFERENCE} CACHE BOOL
    "Build DPA reference implementations" FORCE)
set(BUILD_DOXYGEN ${BUILD_DOXYGEN} CACHE BOOL
    "Build source documentation via doxygen" FORCE)
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING
    "Choose type of build: None Debug Release RelWithDebInfo MinSizeRel" FORCE)

add_subdirectory(src)

