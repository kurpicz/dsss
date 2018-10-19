/*******************************************************************************
 * tests/string_sorting/sequential_stdsort.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include "gtest/gtest.h"

#include "string_sorting/util/algorithm.hpp"
#include "util/random_string_generator.hpp"

namespace dsss::tests::string_sorting {

TEST(std_sort, correctness) {
  constexpr std::size_t number_strings = 10000;
  constexpr std::size_t min_length = 15;
  constexpr std::size_t max_length = 20;
  auto& algorithm_list = dsss::algorithm_list::get_algorithm_list();
  for (const auto& algorithm : algorithm_list) {
    if (algorithm->category() == dsss::algorithm_category::SEQUENTIAL) {
      algorithm->print_info();
      dsss::random_string_set ss(number_strings, min_length, max_length);
      algorithm->run(ss);
      for (std::size_t i = 0; i + 1 < ss.size(); ++i) {
        ASSERT_TRUE(dsss::string_smaller_eq(ss[i], ss[i + 1]));
      }
    }
  }
}

} // namespace dsss::tests::string_sorting

/******************************************************************************/
