################################################################################
# CMakeLists.txt
#
# Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
#
# All rights reserved. Published under the BSD-2 license in the LICENSE file.
################################################################################

cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/misc/cmake")

project(dsss)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(DSSS_OVERSUBSCRIBE
  "Run tests with mpirun [...] --oversubscribe.
  This may be necessary for environments with less than four cores." OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif(NOT CMAKE_BUILD_TYPE)

# TODO: Look for a "modern" (target based) working way
find_package(MPI REQUIRED)
if (MPI_FOUND)
  message(STATUS "MPI FOUND: ${MPI_C_INCLUDE_PATH}")
  include_directories(SYSTEM ${MPI_INCLUDE_PATH})
endif()

add_subdirectory(dsss)
add_subdirectory(benchmark)
add_subdirectory(external)
add_subdirectory(tests)

message(STATUS "Built Type: " ${CMAKE_BUILD_TYPE} )

################################################################################
