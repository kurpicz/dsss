/*******************************************************************************
 * suffix_sorting/index_string.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <cstdint>

#include "util/macros.hpp"
#include "util/string.hpp"

namespace dsss {

template <typename IndexType>
struct indexed_string {
  IndexType index;
  dsss::string string;
} DSSS_ATTRIBUTE_PACKED; // struct indexed_string

template <typename IndexType>
static inline std::size_t string_length(const indexed_string<IndexType>& str) {
  return dsss::string_length(str.string);
}

template <typename IndexType>
static inline std::int64_t string_cmp(const indexed_string<IndexType>& a,
  const indexed_string<IndexType>& b) {
  return dsss::string_cmp(a.string, b.string);
}

template <typename IndexType>
static inline bool string_smaller_eq(const indexed_string<IndexType>& a,
  const indexed_string<IndexType>& b) {
  return dsss::string_smaller_eq(a.string, b.string);
}

} // namespace dsss

/******************************************************************************/
