# -----------------------------------------------------------------------------
# Author:  Garrett Smith
# File:    CMakeLists.cmake
# Created: 08/29/2011
# -----------------------------------------------------------------------------

enable_testing()
include(CTest)

find_package(Boost 1.46.1 COMPONENTS filesystem program_options system thread unit_test_framework)

set(test_libs
    common
    rt
    ${Boost_LIBRARIES}
)

# ------------------------------------------------------------------------------
project(aes_test_vectors)

add_executable(aes_test_vectors aes_test_vectors.cpp ${common_hdr})
target_link_libraries(aes_test_vectors ${test_libs})
add_test(aes_test_vectors ${CMAKE_CURRENT_BINARY_DIR}/aes_test_vectors)

# ------------------------------------------------------------------------------
project(grostl_test_vectors)

add_executable(grostl_test_vectors grostl_test_vectors.cpp ${common_hdr})
target_link_libraries(grostl_test_vectors ${test_libs})
add_test(grostl_test_vectors ${CMAKE_CURRENT_BINARY_DIR}/grostl_test_vectors)

# ------------------------------------------------------------------------------
project(sample_and_hold)

add_executable(sample_and_hold sample_and_hold.cpp ${common_hdr})
target_link_libraries(sample_and_hold ${test_libs})

