/*******************************************************************************
 * string_sorting/sequential/std_sort.cpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include "string_sorting/sequential/std_sort.hpp"
#include "string_sorting/util/algorithm.hpp"

namespace bingmann_qsort {

REGISTER_SEQUENTIAL_SORTER(stdsort,
  "sequential/stdsort",
  "Run std::sort with strcmp() string comparsions")

REGISTER_SEQUENTIAL_SORTER(stdsort1,
  "sequential/stdsort1",
  "Run std::sort with string comparsions (bytewise)")

REGISTER_SEQUENTIAL_SORTER(stdsort4,
  "sequential/stdsort4",
  "Run std::sort with string comparsions (4 bytewise)")

REGISTER_SEQUENTIAL_SORTER(stdsort8,
  "sequential/stdsort8",
  "Run std::sort with string comparsions (8 bytewise)")

} // namespace bingmann_qsort

/******************************************************************************/
