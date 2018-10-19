/*******************************************************************************
 * string_sorting/util/algorithm.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <memory>
#include <set>
#include <type_traits>
#include <vector>

#include "util/string.hpp"
#include "util/string_set.hpp"

namespace dsss {

class sorting_algorithm;

class algorithm_list {
public:
  algorithm_list(const algorithm_list& other) = delete;
  algorithm_list(algorithm_list&& other) = delete;
  void operator = (const algorithm_list& other) = delete;
  void operator = (algorithm_list&& other) = delete;

  inline static algorithm_list& get_algorithm_list() {
    static algorithm_list list;
    return list;
  }

  inline void register_algorithm(sorting_algorithm const* algo) {
    algorithms_.push_back(algo);
  }

  inline auto begin() { return algorithms_.cbegin(); }
  inline auto end() { return algorithms_.cend(); }

private:
  algorithm_list() { }

  // List of static pointers to the different algorithms
  std::vector<sorting_algorithm const*> algorithms_;
}; // class algorithm_list

enum class algorithm_category {
  SEQUENTIAL,
  DISTRIBUTED
}; // enum algorithm_type

class sorting_algorithm {
public:
  sorting_algorithm(std::string name, std::string description)
    : name_(name), description_(description) {
    algorithm_list::get_algorithm_list().register_algorithm(this);
  }

  virtual void run(dsss::string_set& strings) const = 0;
  virtual algorithm_category category() const = 0;

  std::string name() const {
    return name_;
  }

  std::string description() const {
    return description_;
  }

  inline void print_info() const {
    std::cout << name_ << ": " << description_ << std::endl;
  }

private:
  std::string name_;
  std::string description_;
}; // class sorting_algorithm

class sequential_sorter : sorting_algorithm {
  using function_ptr = std::add_pointer<
    void(dsss::string* strings, std::size_t n)>::type;
  
public:
  sequential_sorter(function_ptr sorter, std::string name,
    std::string description) : sorting_algorithm(name, description),
    sorter_(sorter) { }

  void run(dsss::string_set& strings) const override;
  algorithm_category category() const override;

private:
  function_ptr sorter_;
}; // class sequential_sorter

#define REGISTER_SEQUENTIAL_SORTER(sort_func, name, description) \
  static auto _seq_sorting_algorithm_ ## sort_func ## _register \
    = dsss::sequential_sorter(sort_func, name, description);

class distributed_sorter : sorting_algorithm {
  using function_ptr = std::add_pointer<void(dsss::string_set&)>::type;
  
public:
  distributed_sorter(function_ptr sorter, std::string name,
    std::string description) : sorting_algorithm(name, description),
    sorter_(sorter) { }

  void run(dsss::string_set& strings) const override;
  algorithm_category category() const override;

private:
  void prepare_run();
  void concrete_run(dsss::string_set& strings);

private:
  function_ptr sorter_;
}; // class distributed_sorter

#define REGISTER_DISTRIBUTED_SORTER(sort_func, name, description) \
  static auto _dist_sorting_algorithm_ ## sort_func ## _register \
    = dsss::distributed_sorter(sort_func, name, description);

} // namepsace dsss

/******************************************************************************/
