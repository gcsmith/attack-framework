# -----------------------------------------------------------------------------
# Author:  Garrett Smith
# File:    CMakeLists.cmake
# Created: 08/29/2011
# -----------------------------------------------------------------------------

project(common_libs)

find_package(Boost 1.46.1 COMPONENTS filesystem program_options system thread)
include_directories(${Boost_INCLUDE_DIRS})

set(common_hdr
    aes.h
    agilent_reader.h
    attack_engine.h
    attack_manager.h
    des.h
    cmdline.h
    trace.h
    utility.h
)

set(common_src
    aes.cpp
    agilent_reader.cpp
#   attack_cpa.cpp
#   attack_cpow.cpp
#   attack_dpa.cpp
    attack_engine.cpp
    attack_manager.cpp
    des.cpp
    cmdline.cpp
    trace.cpp
    utility.cpp
)

add_library(common ${common_src} ${common_hdr})

