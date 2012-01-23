# -----------------------------------------------------------------------------
# Author:  Garrett Smith
# File:    CMakeLists.cmake
# Created: 08/29/2011
# -----------------------------------------------------------------------------

project(dpacontest_v2)

find_package(PostgreSQL)

if(PostgreSQL_FOUND)
    include_directories(${PostgreSQL_INCLUDE_DIRS})
    link_directories(${PostgreSQL_LIBRARY_DIRS})
endif(PostgreSQL_FOUND)

include_directories(.)
add_definitions(-DHAVE_CONFIG_H)

# -----------------------------------------------------------------------------
set(attack_program_hdr
    attack_last_round.h
    dpa_contest.h
)

set(attack_program_src
    attack_last_round.cpp
    attack_reference.cpp
)

add_executable(attack_program ${attack_program_src} ${attack_program_hdr})

# -----------------------------------------------------------------------------
set(attack_wrapper_hdr
    aes.h
    config.h
    db_postgresql.h
    dpa_contest.h
    dpa_contest_results.h
    trace_files.h
    utils.h
)

set(attack_wrapper_src
    aes.c
    attack_wrapper.c
    db_postgresql.c
    trace_files.c
    utils.c
)

set(attack_wrapper_lib
    ${PostgreSQL_LIBRARIES}
)

if(PostgreSQL_FOUND)
    add_executable(attack_wrapper ${attack_wrapper_src} ${attack_wrapper_hdr})
    target_link_libraries(attack_wrapper ${attack_wrapper_lib})
else(PostgreSQL_FOUND)
    message("WARNING: skipping attack_wrapper. libpq not found")
endif(PostgreSQL_FOUND)

# -----------------------------------------------------------------------------
set(compute_results_hdr
    config.h
    dpa_contest.h
    dpa_contest_results.h
)

set(compute_results_src
    compute_results.c
)

add_executable(compute_results ${compute_results_src} ${compute_results_hdr})

