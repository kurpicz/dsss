################################################################################
# dsss/CMakeLists.txt
#
# Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
#
# All rights reserved. Published under the BSD-2 license in the LICENSE file.
################################################################################

set(DISTRIBUTED_ALGORITHMS_IMPL
  ${PROJECT_SOURCE_DIR}/dsss/string_sorting/distributed/sample_sort.cpp)

add_executable(dss
  ${DISTRIBUTED_ALGORITHMS_IMPL}
  dss.cpp)

target_link_libraries(dss
  tlx
  dsss_string_sorting)

target_compile_options(dss
  PRIVATE
  ${STRING_SORTING_FLAGS}
  $<$<CONFIG:Debug>:${STRING_SORTING_DEBUG_FLAGS}>
  $<$<CONFIG:Release>:${STRING_SORTING_RELEASE_FLAGS}>)

################################################################################
