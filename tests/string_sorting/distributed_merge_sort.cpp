/*******************************************************************************
 * tests/string_sorting/distributed_sample_sort.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include "gtest/gtest.h"

#include "mpi/shift.hpp"
#include "string_sorting/distributed/merge_sort.hpp"
#include "string_sorting/util/algorithm.hpp"
#include "util/string.hpp"

#include "util/random_string_generator.hpp"

namespace dsss::tests::string_sorting {

TEST(sample_sort, correctness) {
  constexpr std::size_t number_strings = 10000;
  constexpr std::size_t min_length = 15;
  constexpr std::size_t max_length = 20;
  auto& algorithm_list = dsss::algorithm_list::get_algorithm_list();
  for (const auto& algorithm : algorithm_list) {
    if (algorithm->category() == dsss::algorithm_category::DISTRIBUTED) {
      algorithm->print_info();
      dsss::random_string_set ss(number_strings, min_length, max_length);
      algorithm->run(ss);

      for (std::size_t i = 0; i + 1 < ss.size(); ++i) {
        ASSERT_TRUE(dsss::string_smaller_eq(ss[i], ss[i + 1]));
      }

      auto smaller_string = dsss::mpi::shift_string_right(ss.back());
      auto larger_string = dsss::mpi::shift_string_left(ss.front());

      dsss::mpi::environment env;
      if (env.rank() > 0) {
        ASSERT_TRUE(dsss::string_smaller_eq(smaller_string.data(), ss.front()));
      }
      if (env.rank() + 1 < env.size()) {
        ASSERT_TRUE(dsss::string_smaller_eq(ss.back(), larger_string.data()));
      }
    }
  }
}

} // namespace dsss::tests::string_sorting

/******************************************************************************/
