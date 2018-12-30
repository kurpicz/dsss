################################################################################
# dsss/CMakeLists.txt
#
# Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
#
# All rights reserved. Published under the BSD-2 license in the LICENSE file.
################################################################################

set(DSSS_MPI_IMPLS
  mpi/environment.cpp)

add_library(dsss_mpi
  ${DSSS_MPI_IMPLS})

target_link_libraries(dsss_mpi
  ${MPI_LIBRARIES}
  dsss_tlx
  ips4o)

target_include_directories(dsss_mpi PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_compile_options(dsss_mpi
  PRIVATE
  ${DSSS_FLAGS}
  $<$<CONFIG:Debug>:${DSSS_DEBUG_FLAGS}>
  $<$<CONFIG:Release>:${DSSS_RELEASE_FLAGS}>)

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
  ${DSSS_STRING_SORTING_IMPLS}
  ${DSSS_UTIL_IMPLS})

target_link_libraries(dsss_string_sorting
  dsss_mpi
  dsss_tlx
  pss)

target_include_directories(dsss_string_sorting PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_compile_options(dsss_string_sorting
  PRIVATE
  ${DSSS_FLAGS}
  $<$<CONFIG:Debug>:${DSSS_DEBUG_FLAGS}>
  $<$<CONFIG:Release>:${DSSS_RELEASE_FLAGS}>)

################################################################################
