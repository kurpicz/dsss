/*******************************************************************************
 * suffix_sorting/border_array.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <array>
#include <type_traits>

#include "mpi/allreduce.hpp"
#include "util/string.hpp"

namespace dsss::suffix_sorting {

template <typename IndexType>
class border_array {

public:
  border_array() {
    std::fill(a_suffixes.begin(), a_suffixes.end(), IndexType(0));
    std::fill(b_suffixes.begin(), b_suffixes.end(), IndexType(0));
  }

  border_array(border_array&&) = default;
  border_array& operator =(border_array&&) = default;

  border_array(const border_array&) = delete;
  border_array& operator =(border_array&) = delete;

  IndexType& a(const dsss::char_type first, const dsss::char_type second) {
    return a_suffixes[first * max_char + second];
  }

  IndexType& a_star(const dsss::char_type first, const dsss::char_type second) {
    return a_suffixes[second * max_char + first];
  }

  IndexType& b(const dsss::char_type first, const dsss::char_type second) {
    return b_suffixes[second * max_char + first];
  }

  IndexType& b_star(const dsss::char_type first, const dsss::char_type second) {
    return b_suffixes[first * max_char + second];
  }

  void communicate() {
    a_suffixes = dsss::mpi::allreduce_sum(a_suffixes);
    b_suffixes = dsss::mpi::allreduce_sum(b_suffixes);
  }

private:
  static constexpr size_t max_char =
    2 + std::numeric_limits<dsss::char_type>::max();

  std::array<IndexType, max_char * max_char> a_suffixes;
  std::array<IndexType, max_char * max_char> b_suffixes;

}; // class border_array

} // namespace dsss::suffix_sorting

/******************************************************************************/
