/*******************************************************************************
 * suffix_sorting/classification.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <algorithm>
#include <cstdint>
#include <tuple>
#include <vector>

#include "mpi/allgather.hpp"
#include "mpi/environment.hpp"
#include "mpi/shift.hpp"
#include "suffix_sorting/border_array.hpp"
#include "util/indexed_string_set.hpp"
#include "util/macros.hpp"
#include "util/string.hpp"
#include "util/string_set.hpp"

namespace dsss::suffix_sorting {

template <typename IndexType>
static std::tuple<dsss::string_set, border_array<IndexType>>
  b_star_substrings_local(std::vector<dsss::char_type>& raw_string) {

  border_array<IndexType> b_array;
  std::vector<IndexType> b_star_pos { 
    static_cast<IndexType>(raw_string.size()) };

  for (std::int64_t i = raw_string.size() - 2; i >= 0;) {
    dsss::char_type c0 = raw_string[i];
    dsss::char_type c1 = raw_string[i + 1];
    if (DSSS_LIKELY(i >= 0)) {
      ++b_array.a_star(c0, c1);
      --i;
    }
    while (i >= 0 && (c0 = raw_string[i]) >= (c1 = raw_string[i + 1])) {
      ++b_array.a(c0, c1);
      --i;
    }
    if (DSSS_LIKELY(i >= 0)) {
      ++b_array.b_star(c0, c1);
      b_star_pos.emplace_back(i--);
      while (i >= 0 && (c0 = raw_string[i]) <= (c1 = raw_string[i + 1])) {
        ++b_array.b(c0, c1);
        --i;
      }
    }
  }
  std::reverse(b_star_pos.begin(), b_star_pos.end());
  // We want to look two characters to the right of each B*-substring, to this
  // end, we add two characters to the string (that we remove later on)
  raw_string.emplace_back(dsss::char_type(0));
  raw_string.emplace_back(dsss::char_type(0));
  std::vector<dsss::char_type> raw_substrings;
  for (size_t i = 0; i + 1 < b_star_pos.size(); ++i) {
    std::copy_n(raw_string.begin() + b_star_pos[i],
      b_star_pos[i + 1] - b_star_pos[i] + 2,
      std::back_inserter(raw_substrings));
  }
  b_star_pos.pop_back();
  // Remove the characters added for a uniform construction of B*-substrings
  raw_string.pop_back();
  raw_string.pop_back();

  return std::forward_as_tuple(
    string_set(std::move(raw_substrings)), std::move(b_array));
}

template <typename IndexType>
static std::tuple<dsss::indexed_string_set<IndexType>, border_array<IndexType>>
  idx_b_star_substrings_local(std::vector<dsss::char_type>& raw_string) {

  border_array<IndexType> b_array;
  std::vector<IndexType> b_star_pos { 
    static_cast<IndexType>(raw_string.size()) };

  for (std::int64_t i = raw_string.size() - 2; i >= 0;) {
    dsss::char_type c0 = raw_string[i];
    dsss::char_type c1 = raw_string[i + 1];
    if (DSSS_LIKELY(i >= 0)) {
      ++b_array.a_star(c0, c1);
      --i;
    }
    while (i >= 0 && (c0 = raw_string[i]) >= (c1 = raw_string[i + 1])) {
      ++b_array.a(c0, c1);
      --i;
    }
    if (DSSS_LIKELY(i >= 0)) {
      ++b_array.b_star(c0, c1);
      b_star_pos.emplace_back(i--);
      while (i >= 0 && (c0 = raw_string[i]) <= (c1 = raw_string[i + 1])) {
        ++b_array.b(c0, c1);
        --i;
      }
    }
  }
  std::reverse(b_star_pos.begin(), b_star_pos.end());
  // We want to look two characters to the right of each B*-substring, to this
  // end, we add two characters to the string (that we remove later on)
  raw_string.emplace_back(dsss::char_type(0));
  raw_string.emplace_back(dsss::char_type(0));
  std::vector<dsss::char_type> raw_substrings;
  for (size_t i = 0; i + 1 < b_star_pos.size(); ++i) {
    std::copy_n(raw_string.begin() + b_star_pos[i],
      b_star_pos[i + 1] - b_star_pos[i] + IndexType(2),
      std::back_inserter(raw_substrings));
  }
  b_star_pos.pop_back();
  // Remove the characters added for a uniform construction of B*-substrings
  raw_string.pop_back();
  raw_string.pop_back();

  return std::forward_as_tuple(
    indexed_string_set<IndexType>(std::move(raw_substrings),
      std::move(b_star_pos)), std::move(b_array));
}

template <typename IndexType>
static std::tuple<dsss::string_set, border_array<IndexType>>
  b_star_substrings(dsss::distributed_string& distributed_raw_string,
    dsss::mpi::environment env = dsss::mpi::environment()) {

  border_array<IndexType> b_array;
  auto& raw_string = distributed_raw_string.string;

  if (env.size() == 1) {
    return b_star_substrings_local<IndexType>(raw_string);
  }

  // This is the position of the first B*-suffix on the PE that we can identify
  // with only the local string. For all PEs except of the last one, there may
  // be another B*-suffix preceding this one.
  std::int64_t pos = raw_string.size() - 2;
  dsss::char_type c0;
  dsss::char_type c1;
  if (env.rank() + 1 < env.size()) {
    // The type of the equal chars depends on the unknown type in the next PE
    while (pos >= 0 && (c0 = raw_string[pos]) <= (c1 = raw_string[pos + 1])) {
      --pos;
    }
    // The next B-type suffix is our first B*-type suffix
    while (pos >= 0 && (c0 = raw_string[pos]) >= (c1 = raw_string[pos + 1])) {
      --pos;
    }
  } else {
    // On the last PE, we know that the last suffix has type A
    ++b_array.a_star(raw_string[pos + 1], dsss::char_type(0));
    while (pos >= 0 && (c0 = raw_string[pos]) >= (c1 = raw_string[pos + 1])) {
      ++b_array.a(c0, c1);
      --pos;
    }
  }
  std::vector<IndexType> b_star_pos;

  std::int64_t first_b_star = 0;
  if (pos >= 0) {
    first_b_star = pos;
    ++b_array.b_star(c0, c1);
    b_star_pos.emplace_back(first_b_star);
    --pos;
  }
  // Make sure that the position (pos) is at an A-suffix
  while (pos >= 0 && (c0 = raw_string[pos]) <= (c1 = raw_string[pos + 1])) {
    ++b_array.b(c0, c1);
    --pos;
  }
  // Identify all B*-substrings and also set pos to position of the leftmost one
  for (std::int64_t i = pos; i >= 0;) {
    if (i >= 0 && (c0 = raw_string[i]) >= (c1 = raw_string[i + 1])) {
      ++b_array.a_star(c0, c1);
      --i; // We know that this has to be an A*-suffix
    }
    while (i >= 0 && (c0 = raw_string[i]) >= (c1 = raw_string[i + 1])) {
      ++b_array.a(c0, c1);
      --i;
    }
    if (DSSS_LIKELY(i >= 0)) {
      ++b_array.b_star(c0, c1);
      b_star_pos.emplace_back(pos = i); // set pos to the B*-position
      --i;
      while (i >= 0 && (c0 = raw_string[i]) <= (c1 = raw_string[i + 1])) {
        ++b_array.b(c0, c1);
        --i;
      }
    }
  }
  std::reverse(std::begin(b_star_pos), std::end(b_star_pos));

  // Next, we shift left the pos-th prefix of the local text,
  // i.e., T[0..pos + 2]
  auto received_chars = dsss::mpi::shift_left(raw_string.data(), pos + 2, env);

  if (env.rank() > 0) {
    // We delete the send symbols (minus the B*-position) from the local string
    // to avoid data duplication ...
    raw_string.erase(raw_string.begin(), raw_string.begin() + pos);
    for (auto& bs_pos : b_star_pos) { bs_pos -= pos; }
    first_b_star -= pos;
  }
  if (env.rank() + 1 < env.size()) {
    const size_t rightoffset = received_chars.size(); 
    // ... we also append the send data to the raw_string
    std::move(received_chars.begin(), received_chars.end(),
      std::back_inserter(raw_string));
    // There is at most one additional B*-substring that needs to be added to
    // the end of our list.
    std::int64_t bs = raw_string.size() - 2;
    //Since the last character corresponds to a B*-suffix, first B-suffixes...
    while (bs > first_b_star &&
      (c0 = raw_string[bs]) <= (c1 = raw_string[bs + 1])) {
      if (bs + rightoffset < raw_string.size()) { ++b_array.b(c0, c1); }
      --bs;
    }
    // ... then A-suffixes ...
    if (bs > first_b_star) {
      if (bs + rightoffset < raw_string.size()) { ++b_array.a_star(c0, c1); }
      --bs;
    }
    while (bs > first_b_star &&
      (c0 = raw_string[bs]) >= (c1 = raw_string[bs + 1])) {
      if (bs + rightoffset < raw_string.size()) { ++b_array.a(c0, c1); }
      --bs;
    }
    // ... then we maybe have found the last B*-suffix at this PE
    if (bs > first_b_star) {
      if (bs + rightoffset < raw_string.size()) { ++b_array.b_star(c0, c1); }
      b_star_pos.emplace_back(bs);
      bs--;
    }
    // ... then we count the rest of the B- and then A-suffixes
    while (bs > first_b_star &&
      (c0 = raw_string[bs]) <= (c1 = raw_string[bs + 1])) {
      if (bs + rightoffset < raw_string.size()) { ++b_array.b(c0, c1); }
      --bs;
    }
    if (bs-- > first_b_star) {  ++b_array.a_star(c0, c1); }
    while (bs > first_b_star &&
      (c0 = raw_string[bs]) >= (c1 = raw_string[bs + 1])) {
      if (bs + rightoffset < raw_string.size()) { ++b_array.a(c0, c1); }
      --bs;
    }
  }

  b_star_pos.emplace_back(raw_string.size() - 2);

  std::vector<dsss::char_type> raw_substrings;
  for (size_t i = 0; i + 1 < b_star_pos.size(); ++i) {
    std::copy_n(raw_string.begin() + b_star_pos[i],
      b_star_pos[i + 1] - b_star_pos[i] + IndexType(2),
      std::back_inserter(raw_substrings));
    raw_substrings.emplace_back(dsss::char_type(0));
  }

  // Delete the last three character, they are the first three characters on the
  // next PE
  raw_string.pop_back();
  raw_string.pop_back();
  raw_string.pop_back();

  b_array.communicate();

  return std::forward_as_tuple(dsss::string_set(std::move(raw_substrings)),
    std::move(b_array));
}

template <typename IndexType>
static std::tuple<dsss::indexed_string_set<IndexType>, border_array<IndexType>>
  idx_b_star_substrings(dsss::distributed_string& distributed_raw_string,
    dsss::mpi::environment env = dsss::mpi::environment()) {

  border_array<IndexType> b_array;
  auto& raw_string = distributed_raw_string.string;

  if (env.size() == 1) {
    return idx_b_star_substrings_local<IndexType>(raw_string);
  }

  // This is the position of the first B*-suffix on the PE that we can identify
  // with only the local string. For all PEs except of the last one, there may
  // be another B*-suffix preceding this one.
  std::int64_t pos = raw_string.size() - 2;
  dsss::char_type c0;
  dsss::char_type c1;
  if (env.rank() + 1 < env.size()) {
    // The type of the equal chars depends on the unknown type in the next PE
    while (pos >= 0 && (c0 = raw_string[pos]) <= (c1 = raw_string[pos + 1])) {
      --pos;
    }
    // The next B-type suffix is our first B*-type suffix
    while (pos >= 0 && (c0 = raw_string[pos]) >= (c1 = raw_string[pos + 1])) {
      --pos;
    }
  } else {
    // On the last PE, we know that the last suffix has type A
    ++b_array.a_star(raw_string[pos + 1], dsss::char_type(0));
    while (pos >= 0 && (c0 = raw_string[pos]) >= (c1 = raw_string[pos + 1])) {
      ++b_array.a(c0, c1);
      --pos;
    }
  }
  std::vector<IndexType> b_star_pos;

  std::int64_t first_b_star = 0;
  if (pos >= 0) {
    first_b_star = pos;
    ++b_array.b_star(c0, c1);
    b_star_pos.emplace_back(first_b_star);
    --pos;
  }
  // Make sure that the position (pos) is at an A-suffix
  while (pos >= 0 && (c0 = raw_string[pos]) <= (c1 = raw_string[pos + 1])) {
    ++b_array.b(c0, c1);
    --pos;
  }
  // Identify all B*-substrings and also set pos to position of the leftmost one
  for (std::int64_t i = pos; i >= 0;) {
    if (i >= 0 && (c0 = raw_string[i]) >= (c1 = raw_string[i + 1])) {
      ++b_array.a_star(c0, c1);
      --i; // We know that this has to be an A*-suffix
    }
    while (i >= 0 && (c0 = raw_string[i]) >= (c1 = raw_string[i + 1])) {
      ++b_array.a(c0, c1);
      --i;
    }
    if (DSSS_LIKELY(i >= 0)) {
      ++b_array.b_star(c0, c1);
      b_star_pos.emplace_back(pos = i); // set pos to the B*-position
      --i;
      while (i >= 0 && (c0 = raw_string[i]) <= (c1 = raw_string[i + 1])) {
        ++b_array.b(c0, c1);
        --i;
      }
    }
  }
  std::reverse(std::begin(b_star_pos), std::end(b_star_pos));

  // Next, we shift left the pos-th prefix of the local text,
  // i.e., T[0..pos + 2]
  auto received_chars = dsss::mpi::shift_left(raw_string.data(), pos + 2, env);

  if (env.rank() > 0) {
    // We delete the send symbols (minus the B*-position) from the local string
    // to avoid data duplication ...
    raw_string.erase(raw_string.begin(), raw_string.begin() + pos);
    for (auto& bs_pos : b_star_pos) { bs_pos -= pos; }
    first_b_star -= pos;
  }
  if (env.rank() + 1 < env.size()) {
    const size_t rightoffset = received_chars.size(); 
    // ... we also append the send data to the raw_string
    std::move(received_chars.begin(), received_chars.end(),
      std::back_inserter(raw_string));
    // There is at most one additional B*-substring that needs to be added to
    // the end of our list.
    std::int64_t bs = raw_string.size() - 2;
    //Since the last character corresponds to a B*-suffix, first B-suffixes...
    while (bs > first_b_star &&
      (c0 = raw_string[bs]) <= (c1 = raw_string[bs + 1])) {
      if (bs + rightoffset < raw_string.size()) { ++b_array.b(c0, c1); }
      --bs;
    }
    // ... then A-suffixes ...
    if (bs > first_b_star) {
      if (bs + rightoffset < raw_string.size()) { ++b_array.a_star(c0, c1); }
      --bs;
    }
    while (bs > first_b_star &&
      (c0 = raw_string[bs]) >= (c1 = raw_string[bs + 1])) {
      if (bs + rightoffset < raw_string.size()) { ++b_array.a(c0, c1); }
      --bs;
    }
    // ... then we maybe have found the last B*-suffix at this PE
    if (bs > first_b_star) {
      if (bs + rightoffset < raw_string.size()) { ++b_array.b_star(c0, c1); }
      b_star_pos.emplace_back(bs);
      bs--;
    }
    // ... then we count the rest of the B- and then A-suffixes
    while (bs > first_b_star &&
      (c0 = raw_string[bs]) <= (c1 = raw_string[bs + 1])) {
      if (bs + rightoffset < raw_string.size()) { ++b_array.b(c0, c1); }
      --bs;
    }
    if (bs-- > first_b_star) {  ++b_array.a_star(c0, c1); }
    while (bs > first_b_star &&
      (c0 = raw_string[bs]) >= (c1 = raw_string[bs + 1])) {
      if (bs + rightoffset < raw_string.size()) { ++b_array.a(c0, c1); }
      --bs;
    }
  }

  b_star_pos.emplace_back(raw_string.size() - 2);

  std::vector<dsss::char_type> raw_substrings;
  for (size_t i = 0; i + 1 < b_star_pos.size(); ++i) {
    std::copy_n(raw_string.begin() + b_star_pos[i],
      b_star_pos[i + 1] - b_star_pos[i] + IndexType(2),
      std::back_inserter(raw_substrings));
    raw_substrings.emplace_back(dsss::char_type(0));
  }

  // Delete the last three character, they are the first three characters on the
  // next PE
  raw_string.pop_back();
  raw_string.pop_back();
  raw_string.pop_back();

  b_array.communicate();

  IndexType offset = (env.rank() == 0) ?
    IndexType(0) : IndexType(distributed_raw_string.offset + pos);

  return std::forward_as_tuple(indexed_string_set<IndexType>(
    std::move(raw_substrings), std::move(b_star_pos), offset), std::move(b_array));
}

} // namespace dsss::suffix_sorting

/******************************************************************************/
