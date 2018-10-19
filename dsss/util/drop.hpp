/*******************************************************************************
 * util/drop.hpp
 *
 * Copyright (C) 2017 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 * Copyright (C) 2017 Marvin Löbel <loebel.marvin@gmail.com>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <type_traits>
#include <utility>

namespace dsss {

template <typename T>
void drop_me(T const&) = delete;

template <typename T>
void drop_me(T&) = delete;

template <typename T>
void drop_me(T const&&) = delete;

template <typename T>
void drop_me(T&& t) {
  std::remove_reference_t<T>(std::move(t));
}

template <typename T, typename... Others>
void drop_me(T const& t, Others const&... others) = delete;

template <typename T, typename... Others>
void drop_me(T& t, Others&... others) = delete;

template <typename T, typename... Others>
void drop_me(T const& t, Others&... others) = delete;

template <typename T, typename... Others>
void drop_me(T& t, Others const&... others) = delete;

template <typename T, typename... Others>
void drop_me(T&& t, Others&&... others) {
  drop_me(std::forward<T>(t));
  drop_me(std::forward<Others...>(others...));
}

} // namespace dsss

/******************************************************************************/
