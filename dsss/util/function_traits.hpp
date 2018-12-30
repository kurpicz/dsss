/*******************************************************************************
 * tests/suffix_sorting/classification.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * Based on https://stackoverflow.com/a/7943765
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <cstdint>
#include <tuple>

namespace dsss::util {

template <typename FunctionType>
struct function_traits
: public function_traits<decltype(&FunctionType::operator())> { };
// struct function_traits

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...) const> {

  using result_type = ReturnType;

  static constexpr size_t arity = sizeof...(Args);

  template <size_t arg_nr>
  struct get {
    using type = typename std::tuple_element<arg_nr, std::tuple<Args...>>::type;
  }; // struct get

}; // function_traits

} // namespace dsss::util

/******************************************************************************/
