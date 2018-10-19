/*******************************************************************************
 * string_sorting/sequential/inssort.cpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include "sequential/inssort.hpp"
#include "string_sorting/util/algorithm.hpp"

namespace inssort {

REGISTER_SEQUENTIAL_SORTER(insertion_sort,
  "sequential/inssort",
  "Insertion sort on strings")

REGISTER_SEQUENTIAL_SORTER(insertion_sort_generic,
  "sequential/inssort_generic",
  "Insertion sort on strings")

} // namespace inssort

/******************************************************************************/
