################################################################################
# dsss/string_sorting/CMakeLists.txt
#
# Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
#
# All rights reserved. Published under the BSD-2 license in the LICENSE file.
################################################################################

set(DSSS_DISTRIBUTED_SORTING
  distributed/sample_sort.cpp)

set(DSSS_SEQUENTIAL_SORTING
  sequential/std_sort.cpp)

set(DSSS_ALL_SORTING
  ${DSSS_DISTRIBUTED_SORTING}
  ${DSSS_DISTRIBUTED_SORTING})

set(DSSS_UTIL
  util/algorithm.cpp)

add_library(dsss_string_sorting OBJECT
  ${DSSS_ALL_SORTING}
  ${DSSS_UTIL})

target_include_directories(dsss_string_sorting PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:>)

################################################################################
