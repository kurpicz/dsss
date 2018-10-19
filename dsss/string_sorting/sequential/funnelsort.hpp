/*******************************************************************************
 * string_sorting/sequential/funnelsort.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <cstdint>

#include "util/string.hpp"

namespace rantala {
  
extern void funnelsort_8way_bfs(dsss::string* strings, std::size_t n);
extern void funnelsort_16way_bfs(dsss::string* strings, std::size_t n);
extern void funnelsort_32way_bfs(dsss::string* strings, std::size_t n);
extern void funnelsort_64way_bfs(dsss::string* strings, std::size_t n);
extern void funnelsort_128way_bfs(dsss::string* strings, std::size_t n);
extern void funnelsort_8way_dfs(dsss::string* strings, std::size_t n);
extern void funnelsort_16way_dfs(dsss::string* strings, std::size_t n);
extern void funnelsort_32way_dfs(dsss::string* strings, std::size_t n);
extern void funnelsort_64way_dfs(dsss::string* strings, std::size_t n);
extern void funnelsort_128way_dfs(dsss::string* strings, std::size_t n);

} // namespace rantala

/******************************************************************************/
