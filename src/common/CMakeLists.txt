# -----------------------------------------------------------------------------
# Author:  Garrett Smith
# File:    CMakeLists.cmake
# Created: 08/29/2011
# -----------------------------------------------------------------------------

project(common_libs)

find_package(Boost 1.46.1 COMPONENTS filesystem program_options system thread)
find_package(Sqlite3 REQUIRED)

include_directories(
    ${Boost_INCLUDE_DIRS}
    ${SQLITE3_INCLUDEDIR}
)

set(common_hdr
    aes.h
    agilent_reader.h
    attack_engine.h
    attack_manager.h
    cmdline.h
    des.h
    grostl.h
    trace.h
    trace_format.h
    trace_format_packed.h
    trace_format_sqlite.h
    trace_format_v1.h
    trace_format_v2.h
    trace_format_v3.h
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
    cmdline.cpp
    des.cpp
    grostl.cpp
    trace.cpp
    trace_format.cpp
    trace_format_packed.cpp
    trace_format_sqlite.cpp
    trace_format_v1.cpp
    trace_format_v2.cpp
    trace_format_v3.cpp
    utility.cpp
)

add_library(common ${common_src} ${common_hdr})

