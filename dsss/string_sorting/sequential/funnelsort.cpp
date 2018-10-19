/*******************************************************************************
 * string_sorting/sequential/funneslsort.cpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include "string_sorting/sequential/funnelsort.hpp"
#include "string_sorting/util/algorithm.hpp"

namespace rantala {

REGISTER_SEQUENTIAL_SORTER(funnelsort_8way_bfs,
  "rantala/funnelsort_8way_bfs",
  "funnelsort 8way bfs")
REGISTER_SEQUENTIAL_SORTER(funnelsort_16way_bfs,
  "rantala/funnelsort_16way_bfs",
  "funnelsort 16way bfs")
REGISTER_SEQUENTIAL_SORTER(funnelsort_32way_bfs,
  "rantala/funnelsort_32way_bfs",
  "funnelsort 32way bfs")
REGISTER_SEQUENTIAL_SORTER(funnelsort_64way_bfs,
  "rantala/funnelsort_64way_bfs",
  "funnelsort 64way bfs")
REGISTER_SEQUENTIAL_SORTER(funnelsort_128way_bfs,
  "rantala/funnelsort_128way_bfs",
  "funnelsort 128way bfs")

REGISTER_SEQUENTIAL_SORTER(funnelsort_8way_dfs,
  "rantala/funnelsort_8way_dfs",
  "funnelsort 8way dfs")
REGISTER_SEQUENTIAL_SORTER(funnelsort_16way_dfs,
  "rantala/funnelsort_16way_dfs",
  "funnelsort 16way dfs")
REGISTER_SEQUENTIAL_SORTER(funnelsort_32way_dfs,
  "rantala/funnelsort_32way_dfs",
  "funnelsort 32way dfs")
REGISTER_SEQUENTIAL_SORTER(funnelsort_64way_dfs,
  "rantala/funnelsort_64way_dfs",
  "funnelsort 64way dfs")
REGISTER_SEQUENTIAL_SORTER(funnelsort_128way_dfs,
  "rantala/funnelsort_128way_dfs",
  "funnelsort 128way dfs")

} // namespace rantala

/******************************************************************************/
