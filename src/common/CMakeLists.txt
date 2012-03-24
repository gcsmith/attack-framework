# -----------------------------------------------------------------------------
# Author:  Garrett Smith
# File:    CMakeLists.cmake
# Created: 08/29/2011
# -----------------------------------------------------------------------------

project(common_libs)

set(common_hdr
    aes.h
    agilent_reader.h
    attack_engine.h
    attack_manager.h
    attack_thread.h
    cmdline.h
    des.h
    grostl.h
    trace.h
    trace_format.h
    trace_format_out.h
    trace_format_packed.h
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
    attack_thread.cpp
    cmdline.cpp
    des.cpp
    grostl.cpp
    trace.cpp
    trace_format.cpp
    trace_format_out.cpp
    trace_format_packed.cpp
    trace_format_v1.cpp
    trace_format_v2.cpp
    trace_format_v3.cpp
    utility.cpp
)

if(SQLITE3_FOUND)
    list(APPEND common_hdr trace_format_sqlite.h)
    list(APPEND common_src trace_format_sqlite.cpp)
endif(SQLITE3_FOUND)

add_library(common ${common_src} ${common_hdr})

