/*******************************************************************************
 * string_sorting/sequential/mergesort.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <cstdint>

#include "util/string.hpp"

namespace rantala {

extern void mergesort_2way(dsss::string*, std::size_t);
extern void mergesort_3way(dsss::string*, std::size_t);
extern void mergesort_4way(dsss::string*, std::size_t);

extern void mergesort_losertree_64way(dsss::string*, std::size_t);
extern void mergesort_losertree_128way(dsss::string*, std::size_t);
extern void mergesort_losertree_256way(dsss::string*, std::size_t);
extern void mergesort_losertree_512way(dsss::string*, std::size_t);
extern void mergesort_losertree_1024way(dsss::string*, std::size_t);

extern void mergesort_2way_unstable(dsss::string*, std::size_t);
extern void mergesort_3way_unstable(dsss::string*, std::size_t);
extern void mergesort_4way_unstable(dsss::string*, std::size_t);

} // namespace rantala

namespace rantala_mergesort_lcp {

extern void mergesort_lcp_2way(dsss::string*, std::size_t);
extern void mergesort_cache1_lcp_2way(dsss::string*, std::size_t);
extern void mergesort_cache2_lcp_2way(dsss::string*, std::size_t);
extern void mergesort_cache4_lcp_2way(dsss::string*, std::size_t);
extern void mergesort_lcp_2way_unstable(dsss::string*, std::size_t);

} // namespace rantala_mergesort_lcp

/******************************************************************************/
