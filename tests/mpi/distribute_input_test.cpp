/*******************************************************************************
 * tests/mpi/distribute_input_test.cpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/
#include "gtest/gtest.h"

#include <fstream>
#include <mpi.h>

#include "mpi/allgather.hpp"
#include "mpi/distribute_input.hpp"
#include "mpi/environment.hpp"

namespace dsss::tests::mpi {

TEST(distribute_string, the_three_brothers_full) {
  dsss::mpi::environment env;

  auto local_slice = dsss::mpi::distribute_string(
    "test_data/the_three_brothers.txt", 0, env);
  const std::size_t string_size = local_slice.string.size();

  auto offset = local_slice.offset;
  auto offsets = dsss::mpi::allgather(offset, env);
  // Check offsets
  if (env.rank() + 1 < env.size()) {
    ASSERT_EQ(local_slice.offset + string_size,
      offsets[env.rank() + 1]);
  }
  if (env.rank() == 0) {
    ASSERT_EQ(0, offsets[0]);
  }

  std::ifstream stream("test_data/the_three_brothers.txt");
  stream.seekg(offset, std::ios::beg);
  std::vector<char> compare_to(string_size);
  stream.read(compare_to.data(), string_size);
  stream.close();

  for (std::size_t i = 0; i < string_size; ++i) {
    ASSERT_EQ(local_slice.string[i], compare_to[i]);
  }
}

} // namespace dsss::tests::mpi

/******************************************************************************/
