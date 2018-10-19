/*******************************************************************************
 * string_sorting/util/string_set.cpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include <utility>

#include "mpi/environment.hpp"
#include "util/string_set.hpp"

namespace dsss {

string_set::string_set() { }

string_set::string_set(std::vector<dsss::char_type>&& string_data)
: strings_raw_data_(std::move(string_data)) {
  strings_.emplace_back(strings_raw_data_.data());
  for (size_t i = 0; i < strings_raw_data_.size();) {
    while (strings_raw_data_[i++] != 0) { }
    strings_.emplace_back(strings_raw_data_.data() + i);
  }
  if constexpr (debug) {
    dsss::mpi::environment env;
    size_t min_length = strings_raw_data_.size();
    size_t max_length = 0;
    for (size_t i = 0; i < strings_.size() - 1; ++i) {
      const size_t cur_length = strings_[i + 1] - strings_[i] - 1;
      min_length = std::min(min_length, cur_length);
      max_length = std::max(max_length, cur_length);
    }
    size_t avg_length = (strings_raw_data_.size() - strings_.size())
      / (strings_.size() - 1);

    for (int32_t rank = 0; rank < env.size(); ++rank) {
      if (rank == env.rank()) {
        std::cout << rank << ": min " << min_length
                  << ", max " << max_length << ", avg " << avg_length
                  << ", count " << strings_.size() - 1 << std::endl;
      }
      env.barrier();
    }
  }
  // Delete pointer to the end of the local strings. No string does start here.
  strings_.pop_back();
}

string_set::string_set(string_set&& other) {
  auto* old_ptr = other.strings_raw_data_.data();
  strings_raw_data_ = std::move(other.strings_raw_data_);
  strings_ = std::move(other.strings_);
  int64_t offset = std::distance(old_ptr, strings_raw_data_.data());

  for (auto& str : strings_) { str -= offset; }
}

string_set& string_set::operator =(string_set&& other) {
  if (this != &other) {
    const auto* old_ptr = other.strings_raw_data_.data();
    strings_raw_data_ = std::move(other.strings_raw_data_);
    strings_ = std::move(other.strings_);
    const auto* new_ptr = strings_raw_data_.data();
    int64_t offset = std::distance(old_ptr, new_ptr);

    for (auto& str : strings_) { str -= offset; }
  }
  return *this;
}

void string_set::update(std::vector<dsss::char_type>&& string_data) {
  strings_raw_data_ = std::move(string_data);
  strings_.clear();
  strings_.emplace_back(strings_raw_data_.data());
  for (size_t i = 0; i < strings_raw_data_.size(); ++i) {
    while (strings_raw_data_[i++] != 0) { }
    strings_.emplace_back(strings_raw_data_.data() + i);
  }
  if constexpr (debug) {
    dsss::mpi::environment env;
    size_t min_length = strings_raw_data_.size();
    size_t max_length = 0;
    for (size_t i = 0; i < strings_.size() - 1; ++i) {
      const size_t cur_length = strings_[i + 1] - strings_[i] - 1;
      min_length = std::min(min_length, cur_length);
      max_length = std::max(max_length, cur_length);
    }
    size_t avg_length = (strings_raw_data_.size() - strings_.size())
      / (strings_.size() - 1);

    for (int32_t rank = 0; rank < env.size(); ++rank) {
      if (rank == env.rank()) {
        std::cout << rank << ": min " << min_length
                  << ", max " << max_length << ", avg " << avg_length
                  << ", count " << strings_.size() - 1 << std::endl;
      }
      env.barrier();
    }
  }
  // Delete pointer to the end of the local strings. No string does start here.
  strings_.pop_back();
}

void string_set::update(std::vector<dsss::string>&& string_data) {
  strings_ = std::move(string_data);
}

dsss::string string_set::operator [](const size_t idx) const {
  return strings_[idx];
}

std::vector<dsss::string>::const_iterator string_set::cbegin() {
  return strings_.cbegin();
}

size_t string_set::size() const { return strings_.size(); }
dsss::string* string_set::strings() { return strings_.data(); }
dsss::string string_set::front() { return strings_.front(); }
dsss::string string_set::back() { return strings_.back(); }
std::vector<dsss::char_type>& string_set::data_container() {
  return strings_raw_data_;
}

dsss::string string_set::raw_data() { return strings_raw_data_.data(); }


} // namespace dsss

/******************************************************************************/
