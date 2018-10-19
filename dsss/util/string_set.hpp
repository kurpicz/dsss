/*******************************************************************************
 * util/string_set.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <cstdint>
#include <vector>

#include "util/string.hpp"

namespace dsss {

class string_set {

  static constexpr bool debug = false;

public:
  string_set();
  string_set(std::vector<dsss::char_type>&& string_data);

  string_set(const string_set&) = delete;
  string_set& operator =(const string_set&) = delete;

  string_set(string_set&& other);
  string_set& operator =(string_set&& other);

  void update(std::vector<dsss::char_type>&& string_data);
  void update(std::vector<dsss::string>&& string_data);


  dsss::string operator [](const std::size_t idx) const;

  std::vector<dsss::string>::const_iterator cbegin();
  std::size_t size() const;
  dsss::string* strings();
  dsss::string front();
  dsss::string back();
  std::vector<dsss::char_type>& data_container();
  dsss::string raw_data();

protected:
  std::vector<dsss::char_type> strings_raw_data_;
  std::vector<dsss::string> strings_;

}; // class string_set

} // namespace dsss

/******************************************************************************/
