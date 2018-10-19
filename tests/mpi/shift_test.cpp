/*******************************************************************************
 * tests/mpi/shift_test.cpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include "gtest/gtest.h"
#include <mpi.h>

#include "mpi/shift.hpp"
#include "mpi/environment.hpp"

#include "util/string_set.hpp"

namespace dsss::tests::mpi {

TEST(shift_right, correctness) {
  dsss::mpi::environment env;

  std::size_t send_value = env.rank();
  std::size_t receive_value = dsss::mpi::shift_right(send_value);

  ASSERT_EQ(receive_value,
    (send_value == 0) ? (env.size() - 1) : send_value - 1);
}

TEST(shift_left, correctness) {
  dsss::mpi::environment env;

  std::size_t send_value = env.rank();
  std::size_t receive_value = dsss::mpi::shift_left(send_value);

  ASSERT_EQ(receive_value,
    (send_value == static_cast<std::size_t>(env.size() - 1)) ?
      0 : send_value + 1);
}

TEST(shift_string_right, correctness) {
  dsss::mpi::environment env;

  std::vector<dsss::char_type> send_data(env.rank() + 1, env.rank() + 1);
  send_data.emplace_back(0);

  std::vector<dsss::char_type> receive_data = dsss::mpi::shift_string_right(
    send_data.data());

  std::int32_t expected_value = ((env.rank() == 0) ? env.size() : env.rank());

  ASSERT_EQ(dsss::string_length(receive_data.data()), expected_value);

  for (std::size_t i = 0; i < dsss::string_length(receive_data.data()); ++i) {
    ASSERT_EQ(receive_data[i], expected_value);
  }
}

TEST(shift_string_left, correctness) {
  dsss::mpi::environment env;

  std::vector<dsss::char_type> send_data(env.rank() + 1, env.rank() + 1);
  send_data.emplace_back(0);

  std::vector<dsss::char_type> receive_data = dsss::mpi::shift_string_left(
    send_data.data());

  std::int32_t expected_value = ((env.rank() + 1 == env.size()) ?
    1 : env.rank() + 2);

  ASSERT_EQ(dsss::string_length(receive_data.data()), expected_value);

  for (std::size_t i = 0; i < dsss::string_length(receive_data.data()); ++i) {
    ASSERT_EQ(receive_data[i], expected_value);
  }
}

} // namespace dsss::tests::mpi

/******************************************************************************/
