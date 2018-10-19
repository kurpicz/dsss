################################################################################
# misc/cmake/FindNUMA.cmake
#
# Copyright (C) 2018 Floran Kurpicz <florian.kurpicz@tu-dortmund.de>
#
# All rights reserved. Published under the BSD-2 license in the LICENSE file.
################################################################################

# Try to find libnuma
#
# The following are set after the configuration is done:
# NUMA_FOUND
# NUMA_INCLUDE_DIRS
# NUMA_DIRS
# NUMA

find_path(NUMA_INCLUDE_DIRS numa /usr/local/include ~/include ~/local/include)
find_library(NUMA numa /usr/local/lib ~/lib ~/local/lib "$ENV{numa_ROOT}")

set(NUMA_FOUND TRUE)

if(NOT NUMA)  
  set(NUMA_FOUND FALSE)
  message("-- Finding NUMA failed")
else()  
  get_filename_component(NUMA_DIRS ${NUMA} PATH)
  message("-- Found NUMA Library: ${NUMA_DIRS}")
  message("-- Found NUMA Include: ${NUMA_INCLUDE_DIRS}")
endif(NOT NUMA)

################################################################################
