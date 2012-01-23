# -----------------------------------------------------------------------------
# Author:  Garrett Smith
# File:    CMakeLists.cmake
# Created: 08/29/2011
# -----------------------------------------------------------------------------

find_package(Boost 1.46.1 COMPONENTS filesystem program_options system thread)

set(test_libs
    common
    rt
    ${Boost_LIBRARIES}
)

# ------------------------------------------------------------------------------
project(sample_and_hold)

add_executable(sample_and_hold sample_and_hold.cpp ${common_hdr})
target_link_libraries(sample_and_hold ${test_libs})

