################################################################################
# dss/external/pss/CMakeLists.txt
#
# Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
#
# All rights reserved. Published under the BSD-2 license in the LICENSE file.
################################################################################

find_package(NUMA REQUIRED)

add_library(pss
  include/rantala/burstsort.cpp
  include/rantala/burstsort2.cpp
  include/rantala/burstsort_mkq.cpp
  include/rantala/funnelsort.cpp
  include/rantala/mergesort.cpp
  include/rantala/mergesort_lcp.cpp
  include/rantala/mergesort_losertree.cpp
  include/rantala/mergesort_unstable.cpp
  include/rantala/multikey_block.cpp
  include/rantala/multikey_cache.cpp
  include/rantala/multikey_dynamic.cpp
  include/rantala/multikey_multipivot.cpp
  include/sequential/bingmann-sample_sort.cpp
  include/sequential/bingmann-sample_sortBSC.cpp
  include/sequential/bingmann-stdsort.cpp
  include/tools/globals.cpp)

target_link_libraries(pss tlx)

target_include_directories(pss PUBLIC include)

target_include_directories(pss PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

################################################################################
