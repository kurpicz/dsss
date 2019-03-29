/*******************************************************************************
 * suffix_sorting/data_structs.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <iostream>

#include "util/macros.hpp"

namespace dsss::suffix_sorting {

template <typename IndexType>
struct bucket_info {
  IndexType starting_position;
  IndexType size;
  IndexType containing;

  bucket_info() = default;
  bucket_info(IndexType _starting_position, IndexType _size,
              IndexType _containing)
    : starting_position(_starting_position), size(_size),
      containing(_containing) { }

  inline IndexType front_pos() const {
    return starting_position + containing;
  }

  inline IndexType back_pos() const {
    return starting_position + size - containing;
  }

  inline size_t remaining() const {
    return size_t(size - containing);
  }
} DSSS_ATTRIBUTE_PACKED;

enum class rank_state : std::uint8_t {
  NONE,
  UNIQUE
}; // enum class state

template <typename IndexType>
struct index_rank {
  IndexType index;
  IndexType rank;

  bool operator < (const index_rank& other) const {
    return rank < other.rank;
  }

  bool operator <= (const index_rank& other) const {
    return rank <= other.rank;
  }

  bool operator == (const index_rank& other) const {
    return rank == other.rank;
  }

  bool operator != (const index_rank& other) const {
    return rank != other.rank;
  }

  index_rank() = default;
  index_rank(IndexType i, IndexType r) : index(i), rank(r) { }

  static constexpr index_rank max() {
    return { std::numeric_limits<IndexType>::max(),
      std::numeric_limits<IndexType>::max() };
  }

  friend std::ostream& operator << (std::ostream& os, const index_rank& ir) {
    return os << "[i=" << ir.index << ", r=" << ir.rank << ']';
  }
} DSSS_ATTRIBUTE_PACKED;

template <typename IndexType>
struct index_rank_state {
  IndexType index;
  IndexType rank;
  rank_state state;

  index_rank_state() = default;
  index_rank_state(IndexType i, IndexType r, rank_state s) : index(i), rank(r),
                                                             state(s) { }

  friend std::ostream& operator << (std::ostream& os,
    const index_rank_state& irs) {
    return os << "[i=" << irs.index << ", r=" << irs.rank << ", s=" <<
      (irs.state == rank_state::NONE ? "NONE" : "UNIQUE") << ']';
  }
} DSSS_ATTRIBUTE_PACKED;

template <typename IndexType>
struct index_rank_rank {
  IndexType index;
  IndexType rank1;
  IndexType rank2;

  bool operator < (const index_rank_rank& other) const {
    return std::tie(rank1, rank2) < std::tie(other.rank1, other.rank2);
  }

  bool operator <= (const index_rank_rank& other) const {
    return std::tie(rank1, rank2) <= std::tie(other.rank1, other.rank2);
  }

  bool operator !=(const index_rank_rank& other) const {
    return std::tie(rank1, rank2) != std::tie(other.rank1, other.rank2);
  }

  bool operator ==(const index_rank_rank& other) const {
    return std::tie(rank1, rank2) == std::tie(other.rank1, other.rank2);
  }

  index_rank_rank() = default;
  index_rank_rank(IndexType i, IndexType r1, IndexType r2) : index(i),
                                                             rank1(r1),
                                                             rank2(r2) { }

  static constexpr index_rank_rank max() {
    return { std::numeric_limits<IndexType>::max(),
      std::numeric_limits<IndexType>::max(),
      std::numeric_limits<IndexType>::max() };
  }

  friend std::ostream& operator << (std::ostream& os,
    const index_rank_rank& irr) {
    return os << "[i=" << irr.index << ", r1=" << irr.rank1 << ", r2=" 
              << irr.rank2 << ']';
  }
} DSSS_ATTRIBUTE_PACKED;

template <typename IndexType>
struct index_rank_rank_state {
  IndexType index;
  IndexType rank1;
  IndexType rank2;
  rank_state state;
} DSSS_ATTRIBUTE_PACKED;

} // namespace dsss::suffix_sorting

/******************************************************************************/
