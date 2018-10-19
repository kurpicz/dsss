/*******************************************************************************
 * string_sorting/sequential/indexed_radix_sort.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 * Based on msd_ce.hpp by Tommi Rantala
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <array>
#include <cstdint>
#include <limits>
#include <utility>

#include "mpi/environment.hpp"

#include "string_sorting/sequential/indexed_inssort.hpp"
#include "util/drop.hpp"
#include "util/indexed_string.hpp"
#include "util/string.hpp"

namespace dsss {

static constexpr std::size_t g_inssort_threshold = 32;

template <typename IndexType,
          std::size_t InssortThreshold = g_inssort_threshold>
static inline void msd_CE0(dsss::indexed_string<IndexType>* strings,
  dsss::indexed_string<IndexType>* sorted, const std::size_t n,
  const std::size_t depth) {

  if (n == 0) { return; }

  if (n < InssortThreshold) { 
    dsss::inssort(strings, n, depth);
    return;
  }

  constexpr std::size_t max_char =
    std::numeric_limits<dsss::char_type>::max() + 1;
  std::array<std::size_t, max_char> bucket_sizes = { 0 };
  for (auto* cur_string = strings; cur_string < strings + n; ++cur_string) {
    ++bucket_sizes[cur_string->string[depth]];
  }

  std::array<dsss::indexed_string<IndexType>*, max_char> buckets;
  buckets[0] = sorted;
  for (std::size_t i = 1; i < max_char; ++i) {
    buckets[i] = buckets[i - 1] + bucket_sizes[i - 1];
  }
  for (auto* cur_string = strings; cur_string < strings + n; ++cur_string) {
    *(buckets[cur_string->string[depth]]++) = *cur_string;
  }
  dsss::drop_me(std::move(buckets));
  std::copy_n(sorted, n, strings);

  auto* bucket_border = strings + bucket_sizes[0];
  for (std::size_t i = 1; i < max_char; ++i) {
    if (bucket_sizes[i] > 0) {
      msd_CE0(bucket_border, sorted, bucket_sizes[i], depth + 1);
      bucket_border += bucket_sizes[i];
    }
  }
}

template <typename IndexType>
static inline void msd_CE0(dsss::indexed_string<IndexType>* strings,
  const std::size_t n) {
  auto* sorted = new dsss::indexed_string<IndexType>[n];
  msd_CE0<IndexType>(strings, sorted, n, 0);
  delete [] sorted;
}

template <typename IndexType>
static inline void msd_CE0(dsss::indexed_string_set<IndexType>& strings) {
  msd_CE0(strings.strings(), strings.size());
}

template <typename IndexType>
static inline void msd_CE2_16bit_5(dsss::indexed_string<IndexType>* strings,
  std::size_t n, std::size_t depth, unsigned char* oracle,
  dsss::indexed_string<IndexType>* sorted) {

  if (n < 32) {
    dsss::inssort(strings, n, depth);
    return;
  }
  std::uint16_t bucket_size[256] = { 0 };
  for (std::size_t i = 0; i < n; ++i) {
    oracle[i] = strings[i].string[depth];
  }
  for (std::size_t i = 0; i < n; ++i) {
    ++bucket_size[oracle[i]];
  }
  std::uint16_t bucket_index[256];
  bucket_index[0] = 0;
  for (std::size_t i = 1; i < 256; ++i) {
    bucket_index[i] = bucket_index[i - 1] + bucket_size[i - 1];
  }
  for (std::size_t i = 0; i < n; ++i) {
    sorted[bucket_index[oracle[i]]++] = strings[i];
  }
  std::copy_n(sorted, n, strings);
  std::size_t start_pos = bucket_size[0];
  for (std::size_t i = 1; i < 256; ++i) {
    if (bucket_size[i] > 0) {
      msd_CE2_16bit_5(strings + start_pos, bucket_size[i], depth + 1, oracle,
        sorted);
      start_pos += bucket_size[i];
    }
  }
}

template <typename IndexType>
static inline void msd_CE6(dsss::indexed_string<IndexType>* strings,
  std::size_t n, std::size_t depth, std::uint16_t* oracle,
  dsss::indexed_string<IndexType>* sorted) {

  if (n < 0x10000) {
    msd_CE2_16bit_5(strings, n, depth, (unsigned char*)oracle,
      sorted);
    return;
  }
  {
    auto get_two_chars = [&](const std::size_t pos, const std::size_t depth) {
      std::uint16_t chars = strings[pos].string[depth];
      if (chars > 0) { chars = (chars << 8) | strings[pos].string[depth + 1]; }
      return chars;
    };

    std::size_t i;
    for (i = 0; i < n - (n % 2); i+=2) {
      oracle[i] = get_two_chars(i, depth);
      oracle[i + 1] = get_two_chars(i + 1, depth);
    }
    for (; i < n; ++i) {
      oracle[i] = get_two_chars(i, depth);
    }
  }
  std::size_t* bucket_size = (std::size_t*)calloc(0x10000, sizeof(std::size_t));
  for (std::size_t i = 0; i < n; ++i) { ++bucket_size[oracle[i]]; }
  std::size_t* bucket_index = new std::size_t[0x10000];
  bucket_index[0] = 0;
  for (std::size_t i = 1; i < 0x10000; ++i) {
    bucket_index[i] = bucket_index[i - 1] + bucket_size[i - 1];
  }
  for (std::size_t i = 0; i < n; ++i) {
    sorted[bucket_index[oracle[i]]++] = strings[i];
  }
  std::copy_n(sorted, n, strings);
  std::size_t start_pos = bucket_size[0];
  for (std::size_t i = 1; i < 256; ++i) {
    if (bucket_size[i] == 0 && i & 0xFF) {
      msd_CE6(strings + start_pos, bucket_size[i], depth + 2, oracle,
        sorted);
    }
    start_pos += bucket_size[i];
  }
  delete [] bucket_size;
  delete [] bucket_index;
}

template <typename IndexType>
static inline void msd_CE6(dsss::indexed_string<IndexType>* strings,
  const std::size_t n) {
  std::uint16_t* oracle = new uint16_t[n];
  auto* sorted = new dsss::indexed_string<IndexType>[n];
  msd_CE6(strings, n, 0, oracle, sorted);
  delete [] oracle;
  delete [] sorted;
}

template <typename IndexType>
static inline void msd_CE6(dsss::indexed_string_set<IndexType>& strings) {
  msd_CE6(strings.strings(), strings.size());
}

} // namesapc dsss

/******************************************************************************/
