cmake_minimum_required(VERSION 3.17.0 FATAL_ERROR)

project(examples)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CI_PROJECT_DIR $ENV{CI_PROJECT_DIR})

cmake_path(GET CI_PROJECT_DIR RELATIVE_PART RELATIVE_CI_PROJECT_DIR)

set(CI_INSTALL_PATH ${CI_PROJECT_DIR}/_staging/${RELATIVE_CI_PROJECT_DIR}/_install)

list(PREPEND CMAKE_SYSTEM_PREFIX_PATH ${CI_INSTALL_PATH})

find_package(LabPlot REQUIRED COMPONENTS SDK)

add_subdirectory(ENSO)
add_subdirectory("NIST - Linear Regression")
add_subdirectory("Helium Rydberg Spectrum")
add_subdirectory("Hertzsprung-Russel Diagram")
add_subdirectory("Logarithmic Scales")
add_subdirectory("Tufte's Minimal Ink Design")
add_subdirectory("Basic Plots")
add_subdirectory("Anatomy of a BoxPlot")
add_subdirectory("Multi-Ranges")
add_subdirectory("Time response of a super regenerative receiver")
add_subdirectory("Data = Smooth + Rough")
add_subdirectory("SOS Morse Signal")
add_subdirectory("Space Debris")
add_subdirectory("Same Data Different Boxplots")
