/*******************************************************************************
 * string_sorting/sequential/sameple_sort.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <cstdint>

#include "util/string.hpp"

namespace rantala_burstsort {
  
extern void burstsort_vector(dsss::string* strings, std::size_t n);
extern void burstsort_brodnik(dsss::string* strings, std::size_t n);
extern void burstsort_bagwell(dsss::string* strings, std::size_t n);
extern void burstsort_vector_block(dsss::string* strings, std::size_t n);
extern void burstsort_superalphabet_vector(dsss::string* strings, std::size_t n);
extern void burstsort_superalphabet_brodnik(dsss::string* strings, std::size_t n);
extern void burstsort_superalphabet_bagwell(dsss::string* strings, std::size_t n);
extern void burstsort_superalphabet_vector_block(dsss::string* strings, std::size_t n);
extern void burstsort_sampling_vector(dsss::string* strings, std::size_t n);
extern void burstsort_sampling_brodnik(dsss::string* strings, std::size_t n);
extern void burstsort_sampling_bagwell(dsss::string* strings, std::size_t n);
extern void burstsort_sampling_vector_block(dsss::string* strings, std::size_t n);
extern void burstsort_sampling_superalphabet_vector(dsss::string* strings, std::size_t n);
extern void burstsort_sampling_superalphabet_brodnik(dsss::string* strings, std::size_t n);
extern void burstsort_sampling_superalphabet_bagwell(dsss::string* strings, std::size_t n);
extern void burstsort_sampling_superalphabet_vector_block(dsss::string* strings, std::size_t n);

} // namespace rantala_burstsort

namespace rantala_burstsort2 {

extern void burstsort2_vector(dsss::string* strings, std::size_t n);
extern void burstsort2_brodnik(dsss::string* strings, std::size_t n);
extern void burstsort2_bagwell(dsss::string* strings, std::size_t n);
extern void burstsort2_vector_block(dsss::string* strings, std::size_t n);
extern void burstsort2_superalphabet_vector(dsss::string* strings, std::size_t n);
extern void burstsort2_superalphabet_brodnik(dsss::string* strings, std::size_t n);
extern void burstsort2_superalphabet_bagwell(dsss::string* strings, std::size_t n);
extern void burstsort2_superalphabet_vector_block(dsss::string* strings, std::size_t n);
extern void burstsort2_sampling_vector(dsss::string* strings, std::size_t n);
extern void burstsort2_sampling_brodnik(dsss::string* strings, std::size_t n);
extern void burstsort2_sampling_bagwell(dsss::string* strings, std::size_t n);
extern void burstsort2_sampling_vector_block(dsss::string* strings, std::size_t n);
extern void burstsort2_sampling_superalphabet_vector(dsss::string* strings, std::size_t n);
extern void burstsort2_sampling_superalphabet_brodnik(dsss::string* strings, std::size_t n);
extern void burstsort2_sampling_superalphabet_bagwell(dsss::string* strings, std::size_t n);
extern void burstsort2_sampling_superalphabet_vector_block(dsss::string* strings, std::size_t n);

} // namespace rantala_burstsort2

namespace rantala_burstsort_mkq {

extern void burstsort_mkq_simpleburst_1(dsss::string*, std::size_t n);
extern void burstsort_mkq_simpleburst_2(dsss::string*, std::size_t n);
extern void burstsort_mkq_simpleburst_4(dsss::string*, std::size_t n);

} // namespace rantala_burstsort_mkq

/******************************************************************************/
