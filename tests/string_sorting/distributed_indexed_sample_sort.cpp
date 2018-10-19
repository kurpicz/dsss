/*******************************************************************************
 * tests/string_sorting/indexed_distributed_sample_sort.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include "gtest/gtest.h"

#include <tuple>

#include "sequential/inssort.hpp"

#include "mpi/allreduce.hpp"
#include "mpi/distribute_input.hpp"
#include "mpi/shift.hpp"
#include "string_sorting/sequential/indexed_inssort.hpp"
#include "string_sorting/sequential/indexed_radix_sort.hpp"
#include "string_sorting/distributed/sample_sort.hpp"
#include "suffix_sorting/classification.hpp"
#include "util/string.hpp"

#include "sequential/bingmann-radix_sort.hpp"

#include "util/random_string_generator.hpp"

namespace dsss::tests::string_sorting {

TEST(indexed_sample_sort_inssort, correctness) {
  constexpr std::size_t number_strings = 10000;
  constexpr std::size_t min_length = 15;
  constexpr std::size_t max_length = 20;
  
  dsss::random_indexed_string_set<std::size_t> ss(
    number_strings, min_length, max_length);

  std::size_t local_size = ss.size();
  std::size_t global_size = dsss::mpi::allreduce_sum(local_size);

  dsss::sample_sort::sample_sort<std::size_t, dsss::inssort<std::size_t>,
    inssort::insertion_sort>(ss);

  ASSERT_TRUE(ss.is_sorted()) << "Strings are not sorted correctly";

  auto smaller_string = dsss::mpi::shift_string_right(ss.back().string);
  auto larger_string = dsss::mpi::shift_string_left(ss.front().string);

  dsss::mpi::environment env;
  if (env.rank() > 0) {
    ASSERT_TRUE(
      dsss::string_smaller_eq(smaller_string.data(), ss.front().string));
  }
  if (env.rank() + 1 < env.size()) {
    ASSERT_TRUE(
      dsss::string_smaller_eq(ss.back().string, larger_string.data()));
  }

  std::size_t new_local_size = ss.size();
  std::size_t new_global_size = dsss::mpi::allreduce_sum(new_local_size);
  ASSERT_EQ(global_size, new_global_size);
}

TEST(indexed_sample_sort_msd_CE0, correctness) {
  constexpr std::size_t number_strings = 10000;
  constexpr std::size_t min_length = 15;
  constexpr std::size_t max_length = 20;
  
  dsss::random_indexed_string_set<std::size_t> ss(
    number_strings, min_length, max_length);

  std::size_t local_size = ss.size();
  std::size_t global_size = dsss::mpi::allreduce_sum(local_size);

  dsss::sample_sort::sample_sort<std::size_t, dsss::msd_CE0<std::size_t>,
    inssort::insertion_sort>(ss);

  ASSERT_TRUE(ss.is_sorted()) << "Strings are not sorted correctly";

  auto smaller_string = dsss::mpi::shift_string_right(ss.back().string);
  auto larger_string = dsss::mpi::shift_string_left(ss.front().string);

  dsss::mpi::environment env;
  if (env.rank() > 0) {
    ASSERT_TRUE(
      dsss::string_smaller_eq(smaller_string.data(), ss.front().string));
  }
  if (env.rank() + 1 < env.size()) {
    ASSERT_TRUE(
      dsss::string_smaller_eq(ss.back().string, larger_string.data()));
  }

  std::size_t new_local_size = ss.size();
  std::size_t new_global_size = dsss::mpi::allreduce_sum(new_local_size);
  ASSERT_EQ(global_size, new_global_size);
}

TEST(indexed_sample_sort_msd_CE6, correctness) {
  constexpr std::size_t number_strings = 10000;
  constexpr std::size_t min_length = 15;
  constexpr std::size_t max_length = 20;
  
  dsss::random_indexed_string_set<std::size_t> ss(
    number_strings, min_length, max_length);

  std::size_t local_size = ss.size();
  std::size_t global_size = dsss::mpi::allreduce_sum(local_size);

  dsss::sample_sort::sample_sort<std::size_t, dsss::msd_CE6<std::size_t>,
    inssort::insertion_sort>(ss);

  ASSERT_TRUE(ss.is_sorted()) << "Strings are not sorted correctly";

  auto smaller_string = dsss::mpi::shift_string_right(ss.back().string);
  auto larger_string = dsss::mpi::shift_string_left(ss.front().string);

  dsss::mpi::environment env;
  if (env.rank() > 0) {
    ASSERT_TRUE(
      dsss::string_smaller_eq(smaller_string.data(), ss.front().string));
  }
  if (env.rank() + 1 < env.size()) {
    ASSERT_TRUE(
      dsss::string_smaller_eq(ss.back().string, larger_string.data()));
  }

  std::size_t new_local_size = ss.size();
  std::size_t new_global_size = dsss::mpi::allreduce_sum(new_local_size);
  ASSERT_EQ(global_size, new_global_size);
}

TEST(indexed_sample_sort_msd_CE0, bs_substrings) {
  dsss::mpi::environment env;

  auto local_slice = dsss::mpi::distribute_string(
    "test_data/the_three_brothers.txt", 0, env);

  dsss::indexed_string_set<std::size_t> bs_substrings;
  std::tie(bs_substrings, std::ignore) = dsss::suffix_sorting::
    idx_b_star_substrings<std::size_t>(local_slice);

  std::size_t local_size = bs_substrings.size();
  std::size_t global_size = dsss::mpi::allreduce_sum(local_size);

  dsss::sample_sort::sample_sort<std::size_t, dsss::msd_CE0<std::size_t>,
    bingmann::bingmann_msd_CE3>(bs_substrings);

  ASSERT_TRUE(bs_substrings.is_sorted());

  auto smaller_string = dsss::mpi::shift_string_right(
    bs_substrings.back().string);
  auto larger_string = dsss::mpi::shift_string_left(
    bs_substrings.front().string);

  if (env.rank() > 0) {
    ASSERT_TRUE(dsss::string_smaller_eq(smaller_string.data(),
      bs_substrings.front().string));
  }
  if (env.rank() + 1 < env.size()) {
    ASSERT_TRUE(dsss::string_smaller_eq(bs_substrings.back().string,
      larger_string.data()));
  }

  std::size_t new_local_size = bs_substrings.size();
  std::size_t new_global_size = dsss::mpi::allreduce_sum(new_local_size);
  ASSERT_EQ(global_size, new_global_size);
}

} // namespace dsss::tests::string_sorting

/******************************************************************************/
