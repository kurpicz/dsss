/*******************************************************************************
 * string_sorting/sequential/mergesort.cpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include "string_sorting/sequential/mergesort.hpp"
#include "string_sorting/util/algorithm.hpp"

namespace rantala {

REGISTER_SEQUENTIAL_SORTER(mergesort_2way,
  "rantala/mergesort_2way",
  "mergesort with 2way merger")
REGISTER_SEQUENTIAL_SORTER(mergesort_3way,
  "rantala/mergesort_3way",
  "mergesort with 3way merger")
REGISTER_SEQUENTIAL_SORTER(mergesort_4way,
  "rantala/mergesort_4way",
  "mergesort with 4way merger")

REGISTER_SEQUENTIAL_SORTER(mergesort_losertree_64way,
  "rantala/mergesort_losertree_64way",
  "mergesort 64way loser tree based")
REGISTER_SEQUENTIAL_SORTER(mergesort_losertree_128way,
  "rantala/mergesort_losertree_128way",
  "mergesort 128way loser tree based")
REGISTER_SEQUENTIAL_SORTER(mergesort_losertree_256way,
  "rantala/mergesort_losertree_256way",
  "mergesort 256way loser tree based")
REGISTER_SEQUENTIAL_SORTER(mergesort_losertree_512way,
  "rantala/mergesort_losertree_512way",
  "mergesort 512way loser tree based")
REGISTER_SEQUENTIAL_SORTER(mergesort_losertree_1024way,
  "rantala/mergesort_losertree_1024way",
  "mergesort 1024way loser tree based")

REGISTER_SEQUENTIAL_SORTER(mergesort_2way_unstable,
  "rantala/mergesort_2way_unstable",
  "mergesort 2way unstable")
REGISTER_SEQUENTIAL_SORTER(mergesort_3way_unstable,
  "rantala/mergesort_3way_unstable",
  "mergesort 3way unstable")
REGISTER_SEQUENTIAL_SORTER(mergesort_4way_unstable,
  "rantala/mergesort_4way_unstable",
  "mergesort 4way unstable")

} // namespace rantala

namespace rantala_mergesort_lcp {

REGISTER_SEQUENTIAL_SORTER(mergesort_lcp_2way,
  "rantala/mergesort_lcp_2way",
  "mergesort LCP with 2way merger")
REGISTER_SEQUENTIAL_SORTER(mergesort_cache1_lcp_2way,
  "rantala/mergesort_cache1_lcp_2way",
  "mergesort LCP with 2way merger and 1byte cache")
REGISTER_SEQUENTIAL_SORTER(mergesort_cache2_lcp_2way,
  "rantala/mergesort_cache2_lcp_2way",
  "mergesort LCP with 2way merger and 2byte cache")
REGISTER_SEQUENTIAL_SORTER(mergesort_cache4_lcp_2way,
  "rantala/mergesort_cache4_lcp_2way",
  "mergesort LCP with 2way merger and 4byte cache")
REGISTER_SEQUENTIAL_SORTER(mergesort_lcp_2way_unstable,
  "rantala/mergesort_lcp_2way_unstable",
  "mergesort Unstable LCP with 2way merger")

} // namespace rantala_mergesort_lcp

/******************************************************************************/
