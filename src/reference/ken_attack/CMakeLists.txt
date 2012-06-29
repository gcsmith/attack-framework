# -----------------------------------------------------------------------------
# Author:  Garrett Smith
# File:    CMakeLists.cmake
# Created: 08/29/2011
# -----------------------------------------------------------------------------

project(reference)

find_package(Java COMPONENTS Development)

set(reference_src
    RandomFileOrderGenerator.java
    ResultsParser.java
    Serialized_to_ASCII.java
    Simulation_Attack.java
    TimingProfiler.java
    Trace.java
    TraceShrinker.java
)

if(Java_Development_FOUND)
    include(UseJava)
    add_jar(reference ${reference_src})
else(Java_Development_FOUND)
    message("WARNING: skipping reference. JDK not found")
endif(Java_Development_FOUND)

