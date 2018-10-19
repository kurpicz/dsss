/*******************************************************************************
 * tests/suffix_sorting/classification.cpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include "gtest/gtest.h"

#include <cstdint>
#include <fstream>
#include <limits>

#include "mpi/distribute_input.hpp"
#include "mpi/environment.hpp"

#include "suffix_sorting/classification.hpp"

namespace dsss::tests::suffix_sorting {

void check_b_star_substrings(std::vector<unsigned char>& original_text,
  dsss::string_set& b_star_substrings) {

  std::size_t string_pos = original_text.size() - 1;
  for (std::int64_t i = b_star_substrings.size() - 1; i >= 0;
    --i, string_pos += 2) {

    const auto cur_bs_substr = b_star_substrings[i];
    const auto substr_length = dsss::string_length(cur_bs_substr);
    ASSERT_GE(substr_length, 2);
    for (std::int64_t j = substr_length - 1; j >= 0; --j) {
      // After a B*, there may follow Bs, but after the first A, there can
      // only follow more As
      bool first_a = false;
      if (j > 0) {
        if (cur_bs_substr[j] > cur_bs_substr[j + 1]) {
          first_a = true;
        }
        if (first_a) {
          ASSERT_GE(cur_bs_substr[j], cur_bs_substr[j + 1]);
        } else {
          ASSERT_LE(cur_bs_substr[j], cur_bs_substr[j + 1]);
        }
      } else {
        ASSERT_LT(cur_bs_substr[j], cur_bs_substr[j + 1]);          
      }
      ASSERT_EQ(original_text[string_pos--], cur_bs_substr[j]);
    }
  }
}

void check_idx_b_star_substrings(std::vector<unsigned char>& original_text,
  dsss::indexed_string_set<std::size_t>& b_star_substrings) {

  std::size_t string_pos = original_text.size() - 1;
  for (std::int64_t i = b_star_substrings.size() - 1; i >= 0; --i) {
    const auto cur_bs_substr = b_star_substrings[i].string;
    const auto substr_length = dsss::string_length(cur_bs_substr);
    ASSERT_GE(substr_length, 2);
    for (std::int64_t j = substr_length - 1; j >= 0; --j) {
      // After a B*, there may follow Bs, but after the first A, there can
      // only follow more As
      bool first_a = false;
      if (j > 0) {
        if (cur_bs_substr[j] > cur_bs_substr[j + 1]) {
          first_a = true;
        }
        if (first_a) {
          ASSERT_GE(cur_bs_substr[j], cur_bs_substr[j + 1]);
        } else {
          ASSERT_LE(cur_bs_substr[j], cur_bs_substr[j + 1]);
        }
      } else {
        ASSERT_LT(cur_bs_substr[j], cur_bs_substr[j + 1]);          
      }
      ASSERT_EQ(original_text[string_pos--], cur_bs_substr[j]);
    }
    ASSERT_EQ(string_pos + 1, b_star_substrings[i].index);
  }
}

void check_borders(std::vector<unsigned char>& original_text,
  dsss::suffix_sorting::border_array<std::size_t>& b_array) {

  dsss::suffix_sorting::border_array<std::size_t> local_b_array;

  ++local_b_array.a_star(original_text.back(), static_cast<dsss::char_type>(0));
  for (std::int64_t i = original_text.size() - 2; i >= 0;) {
    while (i >= 0 && original_text[i] >= original_text[i + 1]) {
      ++local_b_array.a(original_text[i], original_text[i + 1]);
      --i;
    }
    if (i >= 0) {
      ++local_b_array.b_star(original_text[i], original_text[i + 1]);
      --i;
      while (i >= 0 && original_text[i] <= original_text[i + 1]) {
        ++local_b_array.b(original_text[i], original_text[i + 1]);
        --i;
      }
      if (i >= 0) {
        ++local_b_array.a_star(original_text[i], original_text[i + 1]);
        --i;
      }
    }
  }

  for (std::size_t c0 = 0; c0 < 256; ++c0) {
    for (std::size_t c1 = 0; c1 <= c0; ++c1) {
      ASSERT_EQ(local_b_array.a(c0, c1), b_array.a(c0, c1))
        << "c0=" << c0 << " and c1=" << c1;
      if (c0 != c1) {
        ASSERT_EQ(local_b_array.a_star(c0, c1), b_array.a_star(c0, c1))
          << "c0=" << c0 << " and c1=" << c1;;
      }
    }
  }
  for (std::size_t c0 = 0; c0 < 256; ++c0) {
    for (std::size_t c1 = c0; c1 < 256; ++c1) {
      ASSERT_EQ(local_b_array.b(c0, c1), b_array.b(c0, c1))
        << "c0=" << c0 << " and c1=" << c1;
      if (c0 != c1) {
        ASSERT_EQ(local_b_array.b_star(c0, c1), b_array.b_star(c0, c1))
          << "c0=" << c0 << " and c1=" << c1;;
      }
    }
  }

}

TEST(classification, idx_b_star_substrings) {
  dsss::mpi::environment env;

  // Dirstibute the input and compute the classification
  auto local_slice = dsss::mpi::distribute_string(
    "test_data/the_three_brothers.txt", 0, env);

  auto [ bs_substrings, b_array ] = dsss::suffix_sorting::
    idx_b_star_substrings<std::size_t>(local_slice);

  std::ifstream stream("test_data/the_three_brothers.txt");
  stream.ignore(std::numeric_limits<std::streamsize>::max());
  std::size_t file_size = stream.gcount();
  stream.clear();
  stream.seekg(0, std::ios::beg);
  std::vector<unsigned char> compare_to(file_size);
  stream.read(reinterpret_cast<char*>(compare_to.data()), file_size);
  stream.close();

  for (const auto& bs : bs_substrings) {
    for (std::size_t i = 0; i < dsss::string_length(bs.string); ++i) {
      ASSERT_EQ(compare_to[bs.index + i], bs.string[i]);
    }
  }

  auto all_bs_substrings =
    dsss::mpi::allgather_strings(bs_substrings.data_container());

  if (env.rank() == 0) {
    check_b_star_substrings(compare_to, all_bs_substrings);
  }

  if (env.rank() == 0) {
    check_borders(compare_to, b_array);
  }
}

} // namespace dsss::tests::suffix_sorting

/******************************************************************************/
