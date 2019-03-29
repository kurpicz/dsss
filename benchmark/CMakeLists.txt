################################################################################
# dsss/CMakeLists.txt
#
# Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
#
# All rights reserved. Published under the BSD-2 license in the LICENSE file.
################################################################################

set(DISTRIBUTED_ALGORITHMS_IMPL
  ${PROJECT_SOURCE_DIR}/dsss/string_sorting/distributed/merge_sort.cpp)

add_executable(dss
  ${DISTRIBUTED_ALGORITHMS_IMPL}
  dss.cpp)

target_link_libraries(dss
  dsss_string_sorting)

target_compile_options(dss
  PRIVATE
  ${DSSS_FLAGS}
  $<$<CONFIG:Debug>:${DSSS_DEBUG_FLAGS}>
  $<$<CONFIG:Release>:${DSSS_RELEASE_FLAGS}>)


add_executable(dsaca
  dsaca.cpp)

target_link_libraries(dsaca
  dsss_mpi
  dsss_string_sorting
  pss)

target_compile_options(dsaca
  PRIVATE
  ${DSSS_FLAGS}
  $<$<CONFIG:Debug>:${DSSS_DEBUG_FLAGS}>
  $<$<CONFIG:Release>:${DSSS_RELEASE_FLAGS}>)

################################################################################

