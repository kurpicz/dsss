/*******************************************************************************
 * string_sorting/sequential/burstsort.cpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include "string_sorting/sequential/burstsort.hpp"
#include "string_sorting/util/algorithm.hpp"

namespace rantala_burstsort {

// Normal variants
REGISTER_SEQUENTIAL_SORTER(burstsort_vector, "rantala/burstsort_vector",
  "burstsort with std::vector bucket type")
REGISTER_SEQUENTIAL_SORTER(burstsort_brodnik, "rantala/burstsort_brodnik",
  "burstsort with vector_brodnik bucket type")
REGISTER_SEQUENTIAL_SORTER(burstsort_bagwell, "rantala/burstsort_bagwell",
  "burstsort with vector_bagwell bucket type")
REGISTER_SEQUENTIAL_SORTER(burstsort_vector_block,
  "rantala/burstsort_vector_block",
  "burstsort with vector_block bucket type")

// Superalphabet variants
REGISTER_SEQUENTIAL_SORTER(burstsort_superalphabet_vector,
  "rantala/burstsort_superalphabet_vector",
  "burstsort superalphabet with std::vector bucket type")
REGISTER_SEQUENTIAL_SORTER(burstsort_superalphabet_brodnik,
  "rantala/burstsort_superalphabet_brodnik",
  "burstsort superalphabet with vector_brodnik bucket type")
REGISTER_SEQUENTIAL_SORTER(burstsort_superalphabet_bagwell,
  "rantala/burstsort_superalphabet_bagwell",
  "burstsort superalphabet with vector_bagwell bucket type")
REGISTER_SEQUENTIAL_SORTER(burstsort_superalphabet_vector_block,
  "rantala/burstsort_superalphabet_vector_block",
  "burstsort superalphabet with vector_block bucket type")

// Sampling variants - byte alphabet
REGISTER_SEQUENTIAL_SORTER(burstsort_sampling_vector,
  "rantala/burstsort_sampling_vector",
  "burstsort sampling with std::vector bucket type")
REGISTER_SEQUENTIAL_SORTER(burstsort_sampling_brodnik,
  "rantala/burstsort_sampling_brodnik",
  "burstsort sampling with vector_brodnik bucket type")
REGISTER_SEQUENTIAL_SORTER(burstsort_sampling_bagwell,
  "rantala/burstsort_sampling_bagwell",
  "burstsort sampling with vector_bagwell bucket type")
REGISTER_SEQUENTIAL_SORTER(burstsort_sampling_vector_block,
  "rantala/burstsort_sampling_vector_block",
  "burstsort sampling with vector_block bucket type")

// Sampling variants - superalphabet
REGISTER_SEQUENTIAL_SORTER(burstsort_sampling_superalphabet_vector,
  "rantala/burstsort_sampling_superalphabet_vector",
  "burstsort sampling superalphabet with std::vector bucket type")
REGISTER_SEQUENTIAL_SORTER(burstsort_sampling_superalphabet_brodnik,
  "rantala/burstsort_sampling_superalphabet_brodnik",
  "burstsort sampling superalphabet with vector_brodnik bucket type")
REGISTER_SEQUENTIAL_SORTER(burstsort_sampling_superalphabet_bagwell,
  "rantala/burstsort_sampling_superalphabet_bagwell",
  "burstsort sampling superalphabet with vector_bagwell bucket type")
REGISTER_SEQUENTIAL_SORTER(burstsort_sampling_superalphabet_vector_block,
  "rantala/burstsort_sampling_superalphabet_vector_block",
  "burstsort sampling superalphabet with vector_block bucket type")

} // namespace rantala_burstsort

namespace rantala_burstsort2 {

// Normal variants
REGISTER_SEQUENTIAL_SORTER(burstsort2_vector,
  "rantala/burstsort2_vector",
  "burstsort2 with std::vector bucket type")
REGISTER_SEQUENTIAL_SORTER(burstsort2_brodnik,
  "rantala/burstsort2_brodnik",
  "burstsort2 with vector_brodnik bucket type")
REGISTER_SEQUENTIAL_SORTER(burstsort2_bagwell,
  "rantala/burstsort2_bagwell",
  "burstsort2 with vector_bagwell bucket type")
REGISTER_SEQUENTIAL_SORTER(burstsort2_vector_block,
  "rantala/burstsort2_vector_block",
  "burstsort2 with vector_block bucket type")

// Superalphabet variants
REGISTER_SEQUENTIAL_SORTER(burstsort2_superalphabet_vector,
  "rantala/burstsort2_superalphabet_vector",
  "burstsort2 superalphabet with std::vector bucket type")
REGISTER_SEQUENTIAL_SORTER(burstsort2_superalphabet_brodnik,
  "rantala/burstsort2_superalphabet_brodnik",
  "burstsort2 superalphabet with vector_brodnik bucket type")
REGISTER_SEQUENTIAL_SORTER(burstsort2_superalphabet_bagwell,
  "rantala/burstsort2_superalphabet_bagwell",
  "burstsort2 superalphabet with vector_bagwell bucket type")
REGISTER_SEQUENTIAL_SORTER(burstsort2_superalphabet_vector_block,
  "rantala/burstsort2_superalphabet_vector_block",
  "burstsort2 superalphabet with vector_block bucket type")

// Sampling variants - byte alphabet
REGISTER_SEQUENTIAL_SORTER(burstsort2_sampling_vector,
  "rantala/burstsort2_sampling_vector",
  "burstsort2 sampling with std::vector bucket type")
REGISTER_SEQUENTIAL_SORTER(burstsort2_sampling_brodnik,
  "rantala/burstsort2_sampling_brodnik",
  "burstsort2 sampling with vector_brodnik bucket type")
REGISTER_SEQUENTIAL_SORTER(burstsort2_sampling_bagwell,
  "rantala/burstsort2_sampling_bagwell",
  "burstsort2 sampling with vector_bagwell bucket type")
REGISTER_SEQUENTIAL_SORTER(burstsort2_sampling_vector_block,
  "rantala/burstsort2_sampling_vector_block",
  "burstsort2 sampling with vector_block bucket type")

// Sampling variants - superalphabet
REGISTER_SEQUENTIAL_SORTER(burstsort2_sampling_superalphabet_vector,
  "rantala/burstsort2_sampling_superalphabet_vector",
  "burstsort2 sampling superalphabet with std::vector bucket type")
REGISTER_SEQUENTIAL_SORTER(burstsort2_sampling_superalphabet_brodnik,
  "rantala/burstsort2_sampling_superalphabet_brodnik",
  "burstsort2 sampling superalphabet with vector_brodnik bucket type")
REGISTER_SEQUENTIAL_SORTER(burstsort2_sampling_superalphabet_bagwell,
  "rantala/burstsort2_sampling_superalphabet_bagwell",
  "burstsort2 sampling superalphabet with vector_bagwell bucket type")
REGISTER_SEQUENTIAL_SORTER(burstsort2_sampling_superalphabet_vector_block,
  "rantala/burstsort2_sampling_superalphabet_vector_block",
  "burstsort2 sampling superalphabet with vector_block bucket type")

} // namespace rantala_burstsort2

namespace rantala_burstsort_mkq {

REGISTER_SEQUENTIAL_SORTER(burstsort_mkq_simpleburst_1,
  "rantala/burstsort_mkq_simpleburst_1",
  "burstsort_mkq 1byte alphabet with simpleburst")
REGISTER_SEQUENTIAL_SORTER(burstsort_mkq_simpleburst_2,
  "rantala/burstsort_mkq_simpleburst_2",
  "burstsort_mkq 2byte alphabet with simpleburst")
REGISTER_SEQUENTIAL_SORTER(burstsort_mkq_simpleburst_4,
  "rantala/burstsort_mkq_simpleburst_4",
  "burstsort_mkq 4byte alphabet with simpleburst")

} // namespace rantala_burstsort_mkq

/******************************************************************************/
