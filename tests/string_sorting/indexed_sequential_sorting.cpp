/*******************************************************************************
 * tests/string_sorting/indexed_sequential_sorting.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include "gtest/gtest.h"

#include "string_sorting/sequential/indexed_inssort.hpp"
#include "string_sorting/sequential/indexed_radix_sort.hpp"
#include "util/random_string_generator.hpp"

namespace dsss::tests::string_sorting {

TEST(indexed_inssort, correctness) {
  constexpr std::size_t number_strings = 10000;
  constexpr std::size_t min_length = 15;
  constexpr std::size_t max_length = 20;

  random_indexed_string_set<std::size_t> riss(
  number_strings, min_length, max_length);

  dsss::msd_CE0<std::size_t>(riss.strings(), riss.size());

  ASSERT_TRUE(riss.is_sorted());
}

TEST(indexed_msd_CE0, correctness) {
  constexpr std::size_t number_strings = 10000;
  constexpr std::size_t min_length = 15;
  constexpr std::size_t max_length = 20;

  random_indexed_string_set<std::size_t> riss(
    number_strings, min_length, max_length);

  dsss::inssort<std::size_t>(riss.strings(), riss.size());

  ASSERT_TRUE(riss.is_sorted());
}

} // namespace dsss::tests::string_sorting

/******************************************************************************/
