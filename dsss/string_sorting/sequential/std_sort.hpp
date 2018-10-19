/*******************************************************************************
 * string_sorting/sequential/std_sort.cpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <cstdint>

#include "util/string.hpp"

namespace bingmann_qsort {

extern void stdsort(dsss::string* strings, std::size_t n);
extern void stdsort1(dsss::string* strings, std::size_t n);
extern void stdsort4(dsss::string* strings, std::size_t n);
extern void stdsort8(dsss::string* strings, std::size_t n);

} // namespace bingmann_qsort

/******************************************************************************/
