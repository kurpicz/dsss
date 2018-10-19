/*******************************************************************************
 * tests/mpi/allgather_test.cpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include "gtest/gtest.h"
#include <mpi.h>

#include "mpi/allgather.hpp"
#include "mpi/environment.hpp"

#include "util/string_set.hpp"

namespace dsss::tests::mpi {

TEST(allgather, correctness) {
  dsss::mpi::environment env;

  std::size_t rank = env.rank();
  std::vector<size_t> all_ranks = dsss::mpi::allgather(rank);
  for (std::int32_t i = 0; i < env.size(); ++i) {
    ASSERT_EQ(all_ranks[i], i);
  }

  std::size_t const_value = 0;
  std::vector<size_t> all_const_values = dsss::mpi::allgather(const_value);
  for (const auto value : all_const_values) {
    ASSERT_EQ(value, 0);
  }
}

TEST(allgather_vector, correctness) {
  dsss::mpi::environment env;
  std::size_t rank = env.rank();
  std::vector<std::size_t> to_send(10, rank);
  std::vector<std::size_t> all_ranks_tenfold = dsss::mpi::allgatherv(to_send);

  for (std::int32_t rank = 0; rank < env.size(); ++rank) {
    for (std::size_t i = 0; i < 10; ++i) {
      ASSERT_EQ(all_ranks_tenfold[(rank * 10) + i], rank);
    }
  }
}

TEST(allgather_strings, correctness) {
  dsss::mpi::environment env;
  std::vector<dsss::char_type> test_string_data;

  for (std::int64_t i = 0; i < (env.rank() + 1) * 10; ++i) {
    for (std::int64_t j = 0; j < i + 1; ++j) {
      test_string_data.emplace_back((env.rank() % 128) + 1);
    }
    test_string_data.emplace_back(0);
  }
  dsss::string_set send_data(std::move(test_string_data));
  dsss::string_set result = dsss::mpi::allgather_strings(
    send_data.data_container());

  std::int64_t str_offset = 0;
  for (std::int64_t cur_rank = 0; cur_rank < env.size(); ++cur_rank) {
    for (std::int64_t cur_str = 0; cur_str < (cur_rank + 1) * 10; ++cur_str) {
      const std::size_t length =
        dsss::string_length(result[str_offset + cur_str]);
      ASSERT_EQ(length, cur_str + 1);
      for (std::size_t char_pos = 0; char_pos < length; ++char_pos) {
        ASSERT_EQ((cur_rank % 128) + 1, result[str_offset + cur_str][char_pos]);
      }
    }
    str_offset += (cur_rank + 1) * 10;
  }

}

} // namespace dsss::tests::mpi

/******************************************************************************/
