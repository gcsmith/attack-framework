# -----------------------------------------------------------------------------
# Author:  Garrett Smith
# File:    CMakeLists.cmake
# Created: 08/29/2011
# -----------------------------------------------------------------------------

set(boost_components filesystem program_options system thread)

find_package(Boost 1.46.1 COMPONENTS ${boost_components} REQUIRED)
find_package(Sqlite3 REQUIRED)

include_directories(
    ${Boost_INCLUDE_DIRS}
    ${SQLITE3_INCLUDEDIR}
)

set(tool_libs
    common rt
    ${Boost_LIBRARIES}
    ${SQLITE3_LIBRARIES}
)

# ------------------------------------------------------------------------------
project(aestool)

add_executable(aestool aestool.cpp ${common_hdr})
target_link_libraries(aestool ${tool_libs})

# ------------------------------------------------------------------------------
project(convert)

add_executable(convert convert.cpp ${common_hdr})
target_link_libraries(convert ${tool_libs})

# ------------------------------------------------------------------------------
project(destool)

add_executable(destool destool.cpp ${common_hdr})
target_link_libraries(destool ${tool_libs})

# ------------------------------------------------------------------------------
project(grostltool)

add_executable(grostltool grostltool.cpp ${common_hdr})
target_link_libraries(grostltool ${tool_libs})

# ------------------------------------------------------------------------------
project(profile)

add_executable(profile profile.cpp ${common_hdr})
target_link_libraries(profile ${tool_libs})

# ------------------------------------------------------------------------------
project(attack)

add_executable(attack
    attack.cpp
    ../common/attack_cpa.cpp
    ../common/attack_dpa.cpp
    ../common/attack_pscc.cpp
    ../common/attack_relpow.cpp
    ../common/crypto_aes_hd_r0.cpp
    ../common/crypto_aes_hd_r10.cpp
    ../common/crypto_aes_hw_r0.cpp
    ../common/crypto_aes_hw_r10.cpp
    ../common/crypto_des_hd_r0.cpp
    ${common_hdr})
target_link_libraries(attack ${tool_libs})

