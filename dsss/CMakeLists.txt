################################################################################
# dsss/CMakeLists.txt
#
# Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
#
# All rights reserved. Published under the BSD-2 license in the LICENSE file.
################################################################################

set(DSSS_MPI_IMPLS
  mpi/environment.cpp)

set(DSSS_STRING_SORTING_IMPLS
  string_sorting/distributed/merge_sort.cpp
  string_sorting/sequential/burstsort.cpp
  string_sorting/sequential/funnelsort.cpp
  string_sorting/sequential/inssort.cpp
  string_sorting/sequential/mergesort.cpp
  string_sorting/sequential/mkqs.cpp
  string_sorting/sequential/radix_sort.cpp
  string_sorting/sequential/sample_sort.cpp
  string_sorting/sequential/std_sort.cpp
  string_sorting/util/algorithm.cpp)

set(DSSS_UTIL_IMPLS
  util/random_string_generator.cpp
  util/string_set.cpp)

add_library(dsss_string_sorting
  ${DSSS_MPI_IMPLS}
  ${DSSS_STRING_SORTING_IMPLS}
  ${DSSS_UTIL_IMPLS})

target_link_libraries(dsss_string_sorting
  ${MPI_LIBRARIES}
  pss)

target_include_directories(dsss_string_sorting PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

set(STRING_SORTING_FLAGS "-Wall;-pedantic;-Wextra")
set(STRING_SORTING_DEBUG_FLAGS "-O0;-ggdb")
set(STRING_SORTING_RELEASE_FLAGS "-O3;-march=native")

target_compile_options(dsss_string_sorting
  PRIVATE
  ${STRING_SORTING_FLAGS}
  $<$<CONFIG:Debug>:${STRING_SORTING_DEBUG_FLAGS}>
  $<$<CONFIG:Release>:${STRING_SORTING_RELEASE_FLAGS}>)

################################################################################
