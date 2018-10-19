################################################################################
# tests/CMakeLists.txt
#
# Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
#
# All rights reserved. Published under the BSD-2 license in the LICENSE file.
################################################################################

# Copy the test data
set (TEST_DATA
  "test_data/small_text.txt"
  "test_data/the_three_brothers.txt"
  "test_data/the_three_brothers_size_t_sa")
file(COPY ${TEST_DATA} DESTINATION "${CMAKE_BINARY_DIR}/tests/test_data")

include(TestSuit.cmake)

run_mpi_test(mpi/allgather_test)
run_mpi_test(mpi/alltoall_test)
run_mpi_test(mpi/shift_test)
run_mpi_test(mpi/type_mapper_test)

run_mpi_test(string_sorting/distributed_indexed_sample_sort)
run_mpi_test(string_sorting/distributed_sample_sort)

run_mpi_test(suffix_sorting/classification_test)

################################################################################
