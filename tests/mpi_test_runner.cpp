/*******************************************************************************
 * tests/mpi_test_runner.cpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include <cstdint>
#include <mpi.h>

#include "gtest/gtest.h"

int main(int argc, char **argv) {
  std::int32_t result = 0;
  testing::InitGoogleTest(&argc, argv);

  MPI_Init(&argc, &argv);
  result = RUN_ALL_TESTS();
  MPI_Finalize();

  return result;
}

/******************************************************************************/
