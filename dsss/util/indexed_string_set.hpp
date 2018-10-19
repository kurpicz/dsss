/*******************************************************************************
 * util/indexed_string_set.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

#include "util/indexed_string.hpp"

#include "mpi/environment.hpp"

namespace dsss {

template <typename IndexType>
class indexed_string_set {

  static constexpr bool debug = false;

  using idx_string = dsss::indexed_string<IndexType>;

public:
  indexed_string_set() { }

  indexed_string_set(std::vector<dsss::char_type>&& string_data,
    const std::size_t offset) : strings_raw_data_(std::move(string_data)) {
    idxd_strings_.emplace_back(idx_string { offset, strings_raw_data_.data() });
    // i is the position of the character and j is the number of strings already
    // inserted
    for (std::size_t i = 0, j = 1; i < strings_raw_data_.size(); ++i, ++j) {
      while (strings_raw_data_[i++] != 0) { }
      idxd_strings_.emplace_back(offset + i - j, strings_raw_data_.data() + i);
    }
    if constexpr (debug) {
      dsss::mpi::environment env;
      std::size_t min_length = strings_raw_data_.size();
      std::size_t max_length = 0;
      for (std::size_t i = 0; i < idxd_strings_.size() - 1; ++i) {
        const std::size_t cur_length =
          idxd_strings_[i + 1].string - idxd_strings_[i].string - 1;
        min_length = std::min(min_length, cur_length);
        max_length = std::max(max_length, cur_length);
      }
      std::size_t avg_length = (strings_raw_data_.size() - idxd_strings_.size())
        / (idxd_strings_.size() - 1);

      for (int32_t rank = 0; rank < env.size(); ++rank) {
        if (rank == env.rank()) {
          std::cout << rank << ": min " << min_length
                    << ", max " << max_length << ", avg " << avg_length
                    << ", count " << idxd_strings_.size() - 1 << std::endl;
        }
        env.barrier();
      }
    }
    idxd_strings_.pop_back();
  }

  indexed_string_set(std::vector<dsss::char_type>&& string_data,
    std::vector<IndexType>&& indices)
  : strings_raw_data_(std::move(string_data)) {
    if (strings_raw_data_.size() > 0) {
      auto indices_it = indices.cbegin();
      idxd_strings_.emplace_back(idx_string {*indices_it, strings_raw_data_.data() });
      ++indices_it;
      for (std::size_t i = 0; i < strings_raw_data_.size(); ++i, ++indices_it) {
        assert(indices_it != indices.cend());
        while (strings_raw_data_[i++] != 0) { }
        idxd_strings_.emplace_back(idx_string {*indices_it, strings_raw_data_.data() + i });
      }
      if constexpr (debug) {
        dsss::mpi::environment env;
        std::size_t min_length = strings_raw_data_.size();
        std::size_t max_length = 0;
        for (std::size_t i = 0; i < idxd_strings_.size() - 1; ++i) {
          const std::size_t cur_length =
            idxd_strings_[i + 1].string - idxd_strings_[i].string - 1;
          min_length = std::min(min_length, cur_length);
          max_length = std::max(max_length, cur_length);
        }
        std::size_t avg_length = (strings_raw_data_.size() -
          idxd_strings_.size()) / (idxd_strings_.size() - 1);

        for (int32_t rank = 0; rank < env.size(); ++rank) {
          if (rank == env.rank()) {
            std::cout << rank << ": min " << min_length
                      << ", max " << max_length << ", avg " << avg_length
                      << ", count " << idxd_strings_.size() - 1 << std::endl;
          }
          env.barrier();
        }
      }
      idxd_strings_.pop_back();
    }
  }

  indexed_string_set(std::vector<dsss::char_type>&& string_data,
    std::vector<IndexType>&& indices, const IndexType offset)
  : strings_raw_data_(std::move(string_data)) {
    if (strings_raw_data_.size() > 0) {
      auto indices_it = indices.cbegin();
      idxd_strings_.emplace_back(
        idx_string { *indices_it + offset, strings_raw_data_.data() });
      ++indices_it;
      for (std::size_t i = 0; i < strings_raw_data_.size(); ++i, ++indices_it) {
        assert(indices_it != indices.cend());
        while (strings_raw_data_[i++] != 0) { }
        idxd_strings_.emplace_back(
          idx_string { (*indices_it) + offset, strings_raw_data_.data() + i });
      }
      if constexpr (debug) {
        dsss::mpi::environment env;
        std::size_t min_length = strings_raw_data_.size();
        std::size_t max_length = 0;
        for (std::size_t i = 0; i < idxd_strings_.size() - 1; ++i) {
          const std::size_t cur_length =
            idxd_strings_[i + 1].string - idxd_strings_[i].string - 1;
          min_length = std::min(min_length, cur_length);
          max_length = std::max(max_length, cur_length);
        }
        std::size_t avg_length = (strings_raw_data_.size() -
          idxd_strings_.size()) / (idxd_strings_.size() - 1);

        for (int32_t rank = 0; rank < env.size(); ++rank) {
          if (rank == env.rank()) {
            std::cout << rank << ": min " << min_length
                      << ", max " << max_length << ", avg " << avg_length
                      << ", count " << idxd_strings_.size() - 1 << std::endl;
          }
          env.barrier();
        }
      }
      idxd_strings_.pop_back();
    }
  }

  indexed_string_set(const indexed_string_set&) = delete;
  indexed_string_set& operator =(const indexed_string_set&) = delete;

  indexed_string_set(indexed_string_set&& other) {
    auto* old_ptr = other.strings_raw_data_.data();
    strings_raw_data_ = std::move(other.strings_raw_data_);
    idxd_strings_ = std::move(other.idxd_strings_);
    int64_t offset = std::distance(old_ptr, strings_raw_data_.data());

    for (auto& idxd_str : idxd_strings_) { idxd_str.string -= offset; }
  }

  indexed_string_set& operator =(indexed_string_set&& other) {
    if (this != &other) {
      const auto* old_ptr = other.strings_raw_data_.data();
      strings_raw_data_ = std::move(other.strings_raw_data_);
      idxd_strings_ = std::move(other.idxd_strings_);
      const auto* new_ptr = strings_raw_data_.data();
      int64_t offset = std::distance(old_ptr, new_ptr);

      for (auto& idxd_str : idxd_strings_) { idxd_str.string -= offset; }
    }
    return *this;
  }

  void update(std::vector<idx_string>&& string_data) {
    idxd_strings_ = std::move(string_data);
  }

  indexed_string<IndexType> operator [](const size_t idx) const {
    return idxd_strings_[idx];
  }

  size_t size() const {
    return idxd_strings_.size();
  }

  auto cbegin() { return idxd_strings_.cbegin(); }
  auto begin() { return idxd_strings_.begin(); }
  auto end() { return idxd_strings_.end(); }
  auto back() { return idxd_strings_.back(); }
  auto front() { return idxd_strings_.front(); }

  indexed_string<IndexType>* strings() {
    return idxd_strings_.data();
  }

  std::vector<dsss::char_type>& data_container() {
    return strings_raw_data_;
  }

  dsss::string raw_data() {
    return strings_raw_data_.data();
  }

  bool is_sorted() const {
    bool result = true;
    for (size_t i = 0; i + 1 < idxd_strings_.size(); ++i) {
      result &= dsss::string_smaller_eq(idxd_strings_[i], idxd_strings_[i + 1]);
    }
    return result;
  }

protected:
  std::vector<dsss::char_type> strings_raw_data_;
  std::vector<idx_string> idxd_strings_;

}; // class indexed_string_set

} // namespace dsss

/******************************************************************************/
