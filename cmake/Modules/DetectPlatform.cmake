# ------------------------------------------------------------------------------
# Author:  Garrett Smith
# File:    cmake/Modules/DetectPlatform.cmake
# Created: 05/29/2011
# ------------------------------------------------------------------------------

if(WIN32)
    message(STATUS "Building for platform: win32")
    set(MKPROP WIN32)
    set(PLATFORM_WIN32 1)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE -W2)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /TP")
elseif(APPLE)
    message(STATUS "Building for platform: apple")
    set(MKPROP MACOSX_BUNDLE)
    set(PLATFORM_APPLE 1)
    add_definitions(-Wall -Werror)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} rt m)
elseif(UNIX)
    message(STATUS "Building for platform: unix")
    set(MKPROP "")
    set(PLATFORM_UNIX 1)
    add_definitions(-D_GNU_SOURCE -Wall -Werror)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} rt m)
else()
    message(FATAL_ERROR "Unsupported platform detected")
endif()

if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS_PROFILE "-O0 -g -ggdb -pg" CACHE STRING "C flags for profiled builds" FORCE)
    set(CMAKE_CXX_FLAGS_PROFILE "-O0 -g -ggdb -pg" CACHE STRING "C++ flags for profiled builds" FORCE)
    set(CMAKE_EXE_LINKER_FLAGS_PROFILE "" CACHE STRING "" FORCE)
    set(CMAKE_SHARED_LINKER_FLAGS_PROFILE "" CACHE STRING "" FORCE)

    set(CMAKE_C_FLAGS_RELPROFILE "${CMAKE_C_FLAGS_RELWITHDEBINFO} -pg" CACHE STRING "C flags for profiled builds" FORCE)
    set(CMAKE_CXX_FLAGS_RELPROFILE "${CMAKE_C_FLAGS_RELWITHDEBINFO} -pg" CACHE STRING "C++ flags for profiled builds" FORCE)
    set(CMAKE_EXE_LINKER_FLAGS_RELPROFILE "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}" CACHE STRING "" FORCE)
    set(CMAKE_SHARED_LINKER_FLAGS_RELPROFILE "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO}" CACHE STRING "" FORCE)
else()
    set(CMAKE_C_FLAGS_PROFILE "" CACHE STRING "C flags for profiled builds" FORCE)
    set(CMAKE_CXX_FLAGS_PROFILE "" CACHE STRING "C++ flags for profiled builds" FORCE)
    set(CMAKE_EXE_LINKER_FLAGS_PROFILE "" CACHE STRING "" FORCE)
    set(CMAKE_SHARED_LINKER_FLAGS_PROFILE "" CACHE STRING "" FORCE)

    set(CMAKE_C_FLAGS_RELPROFILE "${CMAKE_C_FLAGS_RELWITHDEBINFO} -pg" CACHE STRING "C flags for profiled builds" FORCE)
    set(CMAKE_CXX_FLAGS_RELPROFILE "${CMAKE_C_FLAGS_RELWITHDEBINFO} -pg" CACHE STRING "C++ flags for profiled builds" FORCE)
    set(CMAKE_EXE_LINKER_FLAGS_RELPROFILE "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}" CACHE STRING "" FORCE)
    set(CMAKE_SHARED_LINKER_FLAGS_RELPROFILE "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO}" CACHE STRING "" FORCE)
endif()

