################################################################################
# external/CMakeLists.txt
#
# Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
#
# All rights reserved. Published under the BSD-2 license in the LICENSE file.
################################################################################

add_subdirectory(googletest)
add_subdirectory(pss)

# Add TLX command line parser
add_library(dsss_tlx
  tlx/tlx/cmdline_parser.cpp
  tlx/tlx/digest/sha1.cpp
  tlx/tlx/string/hexdump.cpp
  tlx/tlx/string/parse_si_iec_units.cpp)

target_include_directories(dsss_tlx PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/tlx/
  $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/tlx/)

target_compile_options(dsss_tlx
  PRIVATE
  ${DSSS_FLAGS}
  $<$<CONFIG:Debug>:${DSSS_DEBUG_FLAGS}>
  $<$<CONFIG:Release>:${DSSS_RELEASE_FLAGS}>)

add_library(ips4o INTERFACE)
target_include_directories(ips4o INTERFACE ips4o/ips4o/)

################################################################################
