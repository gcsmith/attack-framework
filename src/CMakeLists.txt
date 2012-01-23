# -----------------------------------------------------------------------------
# Author:  Garrett Smith
# File:    src/CMakeLists.cmake
# Created: 08/29/2011
# -----------------------------------------------------------------------------

add_subdirectory(common)
add_subdirectory(tools)

if(BUILD_REFERENCE)
    add_subdirectory(reference)
endif(BUILD_REFERENCE)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif(BUILD_TESTS)

if(BUILD_DOXYGEN)
    add_subdirectory(doxygen)
endif(BUILD_DOXYGEN)

