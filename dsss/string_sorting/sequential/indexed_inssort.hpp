/*******************************************************************************
 * string_sorting/sequential/indexed_inssort.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <cstdint>

#include "util/indexed_string.hpp"
#include "util/indexed_string_set.hpp"

namespace dsss {

template <typename IndexType>
static inline void inssort(dsss::indexed_string<IndexType>* strings,
  std::size_t n, const std::int64_t depth) {

  if (n == 0) { return; }

  dsss::indexed_string<IndexType>* insert_pos = nullptr;
  dsss::string s;
  dsss::string t;

  for (auto* cmp_pos = strings + 1; --n > 0; ++cmp_pos) {
    const auto insert_str = *cmp_pos;
    for (insert_pos = cmp_pos; insert_pos > strings; --insert_pos) {
      for (s = ((insert_pos - 1)->string + depth),
        t = (insert_str.string  + depth); *s == *t && *s != 0; ++s, ++t) { }
      if (*s <= *t) {
        break;
      }
      *insert_pos = *(insert_pos - 1);
    }
    *insert_pos = insert_str;
  }
}

template <typename IndexType>
static inline void inssort(dsss::indexed_string<IndexType>* strings,
  std::size_t n) {
  inssort(strings, n, 0);
}

template <typename IndexType>
static inline void inssort(dsss::indexed_string_set<IndexType>& strings) {
  inssort(strings.strings(), strings.size());
}

} // namespace dsss

/******************************************************************************/
