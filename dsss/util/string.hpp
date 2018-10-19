/*******************************************************************************
 * string_sorting/util/string.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <cstdint>
#include <iostream>
#include <vector>

namespace dsss {

using char_type = unsigned char;
using string = char_type*;

struct distributed_string {
  size_t offset;
  std::vector<dsss::char_type> string;
}; // struct distributed_string

static inline size_t string_length(const dsss::string str) {
  auto _str = str;
  while (*_str != static_cast<dsss::char_type>(0)) { ++_str; }
  return _str - str;
}

static inline int64_t string_cmp(const dsss::string a, const dsss::string b) {

  auto _a = a;
  auto _b = b;
  while (*_a != static_cast<dsss::char_type>(0) && *_a == *_b) {
    ++_a;
    ++_b;
  }
  return (*_a - *_b);
}

static inline bool string_eq(const dsss::string a, const dsss::string b) {
  return (string_cmp(a, b) == 0);
}

static inline bool string_smaller_eq(const dsss::string a,
  const dsss::string b) {

  return (string_cmp(a, b) <= 0);
}

static inline void string_print(const dsss::string str) {
  auto _str = str;
  while (*_str != static_cast<dsss::char_type>(0)) {
    std::cout << static_cast<size_t>(*(_str++)) << '|';
  }
  std::cout << std::endl;
}

} // namespace dsss

/******************************************************************************/
