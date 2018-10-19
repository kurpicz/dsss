/*******************************************************************************
 * string_sorting/sequential/mkqs.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <cstdint>

#include "util/string.hpp"

namespace rantala {

extern void multikey_cache4(dsss::string*, std::size_t);
extern void multikey_cache8(dsss::string*, std::size_t);

extern void multikey_dynamic_vector1(dsss::string*, std::size_t);
extern void multikey_dynamic_vector2(dsss::string*, std::size_t);
extern void multikey_dynamic_vector4(dsss::string*, std::size_t);
extern void multikey_dynamic_brodnik1(dsss::string*, std::size_t);
extern void multikey_dynamic_brodnik2(dsss::string*, std::size_t);
extern void multikey_dynamic_brodnik4(dsss::string*, std::size_t);
extern void multikey_dynamic_bagwell1(dsss::string*, std::size_t);
extern void multikey_dynamic_bagwell2(dsss::string*, std::size_t);
extern void multikey_dynamic_bagwell4(dsss::string*, std::size_t);
extern void multikey_dynamic_vector_block1(dsss::string*, std::size_t);
extern void multikey_dynamic_vector_block2(dsss::string*, std::size_t);
extern void multikey_dynamic_vector_block4(dsss::string*, std::size_t);

extern void multikey_multipivot_brute_simd1(dsss::string*, std::size_t);
extern void multikey_multipivot_brute_simd2(dsss::string*, std::size_t);
extern void multikey_multipivot_brute_simd4(dsss::string*, std::size_t);

} // namespace rantala

namespace rantala_multikey_block {

extern void multikey_block1(dsss::string*, std::size_t);
extern void multikey_block2(dsss::string*, std::size_t);
extern void multikey_block4(dsss::string*, std::size_t);
  
} // namespace rantala_multikey_block

/******************************************************************************/
