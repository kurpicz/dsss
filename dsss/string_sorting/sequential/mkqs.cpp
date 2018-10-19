/*******************************************************************************
 * string_sorting/sequential/mkqs.cpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include "sequential/bingmann-mkqs.hpp"
#include "sequential/bs-mkqs.hpp"

#include "mkqs.hpp"
#include "string_sorting/util/algorithm.hpp"

namespace bingmann {

REGISTER_SEQUENTIAL_SORTER(mkqs_generic,
  "sequential/mkqs_generic",
  "Generic Multi-key quicksort")

} // namespace bingmann

namespace bs_mkqs {

REGISTER_SEQUENTIAL_SORTER(bs_mkqsort,
  "bs/mkqsort",
  "bs_mkqs Original Multikey-Quicksort")

} // namespace bs_mkqs

namespace rantala {

REGISTER_SEQUENTIAL_SORTER(multikey_cache4,
  "rantala/multikey_cache4",
  "multikey_cache with 4byte cache")
REGISTER_SEQUENTIAL_SORTER(multikey_cache8,
  "rantala/multikey_cache8",
  "multikey_cache with 8byte cache")

REGISTER_SEQUENTIAL_SORTER(multikey_dynamic_vector1,
  "rantala/multikey_dynamic_vector1",
  "multikey_dynamic with std::vector bucket type and 1byte alphabet")
REGISTER_SEQUENTIAL_SORTER(multikey_dynamic_vector2,
  "rantala/multikey_dynamic_vector2",
  "multikey_dynamic with std::vector bucket type and 2byte alphabet")
REGISTER_SEQUENTIAL_SORTER(multikey_dynamic_vector4,
  "rantala/multikey_dynamic_vector4",
  "multikey_dynamic with std::vector bucket type and 4byte alphabet")

REGISTER_SEQUENTIAL_SORTER(multikey_dynamic_brodnik1,
  "rantala/multikey_dynamic_brodnik1",
  "multikey_dynamic with vector_brodnik bucket type and 1byte alphabet")
REGISTER_SEQUENTIAL_SORTER(multikey_dynamic_brodnik2,
  "rantala/multikey_dynamic_brodnik2",
  "multikey_dynamic with vector_brodnik bucket type and 2byte alphabet")
REGISTER_SEQUENTIAL_SORTER(multikey_dynamic_brodnik4,
  "rantala/multikey_dynamic_brodnik4",
  "multikey_dynamic with vector_brodnik bucket type and 4byte alphabet")

REGISTER_SEQUENTIAL_SORTER(multikey_dynamic_bagwell1,
  "rantala/multikey_dynamic_bagwell1",
  "multikey_dynamic with vector_bagwell bucket type and 1byte alphabet")
REGISTER_SEQUENTIAL_SORTER(multikey_dynamic_bagwell2,
  "rantala/multikey_dynamic_bagwell2",
  "multikey_dynamic with vector_bagwell bucket type and 2byte alphabet")
REGISTER_SEQUENTIAL_SORTER(multikey_dynamic_bagwell4,
  "rantala/multikey_dynamic_bagwell4",
  "multikey_dynamic with vector_bagwell bucket type and 4byte alphabet")

REGISTER_SEQUENTIAL_SORTER(multikey_dynamic_vector_block1,
  "rantala/multikey_dynamic_vector_block1",
  "multikey_dynamic with vector_block bucket type and 1byte alphabet")
REGISTER_SEQUENTIAL_SORTER(multikey_dynamic_vector_block2,
  "rantala/multikey_dynamic_vector_block2",
  "multikey_dynamic with vector_block bucket type and 2byte alphabet")
REGISTER_SEQUENTIAL_SORTER(multikey_dynamic_vector_block4,
  "rantala/multikey_dynamic_vector_block4",
  "multikey_dynamic with vector_block bucket type and 4byte alphabet")

REGISTER_SEQUENTIAL_SORTER(multikey_multipivot_brute_simd1,
  "rantala/multikey_multipivot_brute_simd1",
  "multikey_multipivot brute_simd with 1byte alphabet")
REGISTER_SEQUENTIAL_SORTER(multikey_multipivot_brute_simd2,
  "rantala/multikey_multipivot_brute_simd2",
  "multikey_multipivot brute_simd with 2byte alphabet")
REGISTER_SEQUENTIAL_SORTER(multikey_multipivot_brute_simd4,
  "rantala/multikey_multipivot_brute_simd4",
  "multikey_multipivot brute_simd with 4byte alphabet")

} // namespace rantala

namespace rantala_multikey_block {

REGISTER_SEQUENTIAL_SORTER(multikey_block1,
  "rantala/multikey_block1",
  "multikey_block with 1byte alphabet")
REGISTER_SEQUENTIAL_SORTER(multikey_block2,
  "rantala/multikey_block2",
  "multikey_block with 2byte alphabet")
REGISTER_SEQUENTIAL_SORTER(multikey_block4,
  "rantala/multikey_block4",
  "multikey_block with 4byte alphabet")

} // namespace rantala_multikey_block



/******************************************************************************/
