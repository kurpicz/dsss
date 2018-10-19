/*******************************************************************************
 * tests/util/random_string_generator.cpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include <algorithm>

#include <iostream>

#include "util/random_string_generator.hpp"

namespace dsss {

random_string_set::random_string_set(const size_t size,
  const size_t alphabet_size) {
  std::random_device rand_seed;
  std::mt19937 rand_gen(rand_seed());
  std::uniform_int_distribution<dsss::char_type> char_dis(1,
    std::min<size_t>(
      std::numeric_limits<dsss::char_type>::max(), alphabet_size + 1));
  strings_raw_data_.reserve(size + 1);
  for (size_t i = 0; i < size; ++i) {
    strings_raw_data_.emplace_back(char_dis(rand_gen));
  }
  strings_raw_data_.emplace_back(dsss::char_type(0));
  strings_.emplace_back(strings_raw_data_.data());
}

random_string_set::random_string_set(const size_t size,
  const size_t min_length, const size_t max_length) {

  std::random_device rand_seed;
  std::mt19937 rand_gen(rand_seed());
  std::uniform_int_distribution<size_t> length_dis(min_length, max_length);
  std::uniform_int_distribution<dsss::char_type> char_dis(1,
    std::numeric_limits<dsss::char_type>::max());

  for (size_t i = 0; i < size; ++i) {
    const size_t string_length = length_dis(rand_gen);
    for (size_t j = 0; j < string_length; ++j) {
      strings_raw_data_.emplace_back(char_dis(rand_gen));
    }
    strings_raw_data_.emplace_back(dsss::char_type(0));
  }

  strings_.emplace_back(strings_raw_data_.data());
  for (size_t i = 0; i < strings_raw_data_.size(); ++i) {
    while (strings_raw_data_[i++] != 0) { }
    strings_.emplace_back(strings_raw_data_.data() + i);
  }
  strings_.pop_back();
}

} // namespace dsss

/******************************************************************************/
