/*******************************************************************************
 * string_sorting/distributed/sample_sort.cpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include "rantala/msd_a.hpp"
#include "rantala/msd_a2.hpp"
#include "rantala/msd_ce.hpp"
#include "rantala/msd_ci.hpp"
#include "rantala/msd_dyn_block.hpp"
#include "rantala/msd_dyn_vector.hpp"
#include "rantala/msd_lsd.hpp"

#include "sequential/bingmann-mkqs.hpp"
#include "sequential/bingmann-radix_sort.hpp"
#include "sequential/bs-mkqs.hpp"
#include "sequential/inssort.hpp"

#include "string_sorting/distributed/sample_sort.hpp"
#include "string_sorting/sequential/burstsort.hpp"
#include "string_sorting/sequential/funnelsort.hpp"
#include "string_sorting/sequential/mergesort.hpp"
#include "string_sorting/sequential/mkqs.hpp"
#include "string_sorting/sequential/sample_sort.hpp"
#include "string_sorting/sequential/std_sort.hpp"
#include "string_sorting/util/algorithm.hpp"

namespace dsss::sample_sort {

#define BUILD_SAMPLE_SORT(namespace, sequential)                               \
void sample_sort_##sequential(dsss::string_set& local_string_set) {            \
  sample_sort<namespace::sequential>(local_string_set);                        \
}                                                                              \
REGISTER_DISTRIBUTED_SORTER(sample_sort_##sequential,                          \
  "distributed/sample_sort"#sequential,                                        \
  "Run distributed sample sort using " #sequential " for local sorting.")

/*******************************************************************************
 * Register insertion sort variants as local string sorter within our
 * distributed sample sort.
 ******************************************************************************/

  // TODO: VERY SLOW, add cmake option to include this one
  // BUILD_SAMPLE_SORT(inssort, insertion_sort)
  // BUILD_SAMPLE_SORT(inssort, insertion_sort_generic)

/*******************************************************************************
 * Register multi-key quicksort variants as local string sorter within our
 * distributed sample sort.
 ******************************************************************************/

  BUILD_SAMPLE_SORT(bingmann, mkqs_generic)
  BUILD_SAMPLE_SORT(bs_mkqs, bs_mkqsort)
  BUILD_SAMPLE_SORT(rantala, multikey_cache4)
  BUILD_SAMPLE_SORT(rantala, multikey_cache8)
  BUILD_SAMPLE_SORT(rantala, multikey_dynamic_vector1)
  BUILD_SAMPLE_SORT(rantala, multikey_dynamic_vector2)
  BUILD_SAMPLE_SORT(rantala, multikey_dynamic_vector4)
  BUILD_SAMPLE_SORT(rantala, multikey_dynamic_brodnik1)
  BUILD_SAMPLE_SORT(rantala, multikey_dynamic_brodnik2)
  BUILD_SAMPLE_SORT(rantala, multikey_dynamic_brodnik4)
  BUILD_SAMPLE_SORT(rantala, multikey_dynamic_bagwell1)
  BUILD_SAMPLE_SORT(rantala, multikey_dynamic_bagwell2)
  BUILD_SAMPLE_SORT(rantala, multikey_dynamic_bagwell4)
  BUILD_SAMPLE_SORT(rantala, multikey_dynamic_vector_block1)
  BUILD_SAMPLE_SORT(rantala, multikey_dynamic_vector_block2)
  BUILD_SAMPLE_SORT(rantala, multikey_dynamic_vector_block4)
  BUILD_SAMPLE_SORT(rantala, multikey_multipivot_brute_simd1)
  BUILD_SAMPLE_SORT(rantala, multikey_multipivot_brute_simd2)
  BUILD_SAMPLE_SORT(rantala, multikey_multipivot_brute_simd4)

  BUILD_SAMPLE_SORT(rantala_multikey_block, multikey_block1)
  BUILD_SAMPLE_SORT(rantala_multikey_block, multikey_block2)
  BUILD_SAMPLE_SORT(rantala_multikey_block, multikey_block4)

/*******************************************************************************
 * Register radix sort variants as local string sorter within our distributed
 * sample sort.
 ******************************************************************************/

  BUILD_SAMPLE_SORT(bingmann, bingmann_msd_CE0)
  BUILD_SAMPLE_SORT(bingmann, bingmann_msd_CE0_generic)
  BUILD_SAMPLE_SORT(bingmann, bingmann_msd_CE1)
  BUILD_SAMPLE_SORT(bingmann, bingmann_msd_CE2)
  BUILD_SAMPLE_SORT(bingmann, bingmann_msd_CE3)
  BUILD_SAMPLE_SORT(bingmann, bingmann_msd_CI0)
  BUILD_SAMPLE_SORT(bingmann, bingmann_msd_CI1)
  BUILD_SAMPLE_SORT(bingmann, bingmann_msd_CI2)
  BUILD_SAMPLE_SORT(bingmann, bingmann_msd_CI2_generic)
  BUILD_SAMPLE_SORT(bingmann, bingmann_msd_CI3)
  BUILD_SAMPLE_SORT(bingmann, bingmann_msd_CE0_sb)
  BUILD_SAMPLE_SORT(bingmann, bingmann_msd_CE2_sb)
  BUILD_SAMPLE_SORT(bingmann, bingmann_msd_CE3_sb)
  BUILD_SAMPLE_SORT(bingmann, bingmann_msd_CI2_sb)
  BUILD_SAMPLE_SORT(bingmann, bingmann_msd_CI3_sb)

  BUILD_SAMPLE_SORT(rantala_msd_a, msd_A)
  BUILD_SAMPLE_SORT(rantala_msd_a, msd_A_adaptive)

  BUILD_SAMPLE_SORT(rantala_msd_a2, msd_A2)
  BUILD_SAMPLE_SORT(rantala_msd_a2, msd_A2_adaptive)

  BUILD_SAMPLE_SORT(rantala, msd_CE0)
  BUILD_SAMPLE_SORT(rantala, msd_CE1)
  BUILD_SAMPLE_SORT(rantala, msd_CE2)
  BUILD_SAMPLE_SORT(rantala, msd_CE3)
  BUILD_SAMPLE_SORT(rantala, msd_CE4)
  BUILD_SAMPLE_SORT(rantala, msd_CE5)
  BUILD_SAMPLE_SORT(rantala, msd_CE6)
  BUILD_SAMPLE_SORT(rantala, msd_CE7)
  BUILD_SAMPLE_SORT(rantala, msd_ci)
  BUILD_SAMPLE_SORT(rantala, msd_ci_adaptive)
  BUILD_SAMPLE_SORT(rantala, msd_A_lsd4)
  BUILD_SAMPLE_SORT(rantala, msd_A_lsd6)
  BUILD_SAMPLE_SORT(rantala, msd_A_lsd8)
  BUILD_SAMPLE_SORT(rantala, msd_A_lsd10)
  BUILD_SAMPLE_SORT(rantala, msd_A_lsd12)
  BUILD_SAMPLE_SORT(rantala, msd_A_lsd_adaptive4)
  BUILD_SAMPLE_SORT(rantala, msd_A_lsd_adaptive6)
  BUILD_SAMPLE_SORT(rantala, msd_A_lsd_adaptive8)
  BUILD_SAMPLE_SORT(rantala, msd_A_lsd_adaptive10)
  BUILD_SAMPLE_SORT(rantala, msd_A_lsd_adaptive12)

  BUILD_SAMPLE_SORT(rantala_msd_db, msd_DB)

/*******************************************************************************
 * Register sample sort variants as local string sorter within our distributed
 * sample sort.
 ******************************************************************************/

  BUILD_SAMPLE_SORT(bingmann_qsort, stdsort)
  BUILD_SAMPLE_SORT(bingmann_sample_sortBSC_original, bingmann_sample_sortBSC_original)
  BUILD_SAMPLE_SORT(bingmann_sample_sortBSC_original, bingmann_sample_sortBSCA_original)
  BUILD_SAMPLE_SORT(bingmann_sample_sortBSC_original, bingmann_sample_sortBSCE_original)
  BUILD_SAMPLE_SORT(bingmann_sample_sortBSC_original, bingmann_sample_sortBSCEA_original)

  BUILD_SAMPLE_SORT(bingmann_sample_sort, bingmann_sample_sortBSC)
  BUILD_SAMPLE_SORT(bingmann_sample_sort, bingmann_sample_sortBTC)
  BUILD_SAMPLE_SORT(bingmann_sample_sort, bingmann_sample_sortBTCA)
  BUILD_SAMPLE_SORT(bingmann_sample_sort, bingmann_sample_sortBTCU)
  BUILD_SAMPLE_SORT(bingmann_sample_sort, bingmann_sample_sortBTCUI)
  BUILD_SAMPLE_SORT(bingmann_sample_sort, bingmann_sample_sortBTCT)
  BUILD_SAMPLE_SORT(bingmann_sample_sort, bingmann_sample_sortBTCTU)
  BUILD_SAMPLE_SORT(bingmann_sample_sort, bingmann_sample_sortBTCTUI)
  BUILD_SAMPLE_SORT(bingmann_sample_sort, bingmann_sample_sortBTCE)
  BUILD_SAMPLE_SORT(bingmann_sample_sort, bingmann_sample_sortBTCEA)
  BUILD_SAMPLE_SORT(bingmann_sample_sort, bingmann_sample_sortBTCEU)
  BUILD_SAMPLE_SORT(bingmann_sample_sort, bingmann_sample_sortBTCEV)

/*******************************************************************************
 * Register burstsort variants as local string sorter within our distributed
 * sample sort.
 ******************************************************************************/

  BUILD_SAMPLE_SORT(rantala_burstsort, burstsort_vector)
  BUILD_SAMPLE_SORT(rantala_burstsort, burstsort_brodnik)
  BUILD_SAMPLE_SORT(rantala_burstsort, burstsort_bagwell)
  BUILD_SAMPLE_SORT(rantala_burstsort, burstsort_vector_block)
  BUILD_SAMPLE_SORT(rantala_burstsort, burstsort_superalphabet_vector)
  BUILD_SAMPLE_SORT(rantala_burstsort, burstsort_superalphabet_brodnik)
  BUILD_SAMPLE_SORT(rantala_burstsort, burstsort_superalphabet_bagwell)
  BUILD_SAMPLE_SORT(rantala_burstsort, burstsort_superalphabet_vector_block)
  BUILD_SAMPLE_SORT(rantala_burstsort, burstsort_sampling_vector)
  BUILD_SAMPLE_SORT(rantala_burstsort, burstsort_sampling_brodnik)
  BUILD_SAMPLE_SORT(rantala_burstsort, burstsort_sampling_bagwell)
  BUILD_SAMPLE_SORT(rantala_burstsort, burstsort_sampling_vector_block)
  BUILD_SAMPLE_SORT(rantala_burstsort, burstsort_sampling_superalphabet_vector)
  BUILD_SAMPLE_SORT(rantala_burstsort, burstsort_sampling_superalphabet_brodnik)
  BUILD_SAMPLE_SORT(rantala_burstsort, burstsort_sampling_superalphabet_bagwell)
  BUILD_SAMPLE_SORT(rantala_burstsort, burstsort_sampling_superalphabet_vector_block)

  BUILD_SAMPLE_SORT(rantala_burstsort2, burstsort2_vector)
  BUILD_SAMPLE_SORT(rantala_burstsort2, burstsort2_brodnik)
  BUILD_SAMPLE_SORT(rantala_burstsort2, burstsort2_bagwell)
  BUILD_SAMPLE_SORT(rantala_burstsort2, burstsort2_vector_block)
  BUILD_SAMPLE_SORT(rantala_burstsort2, burstsort2_superalphabet_vector)
  BUILD_SAMPLE_SORT(rantala_burstsort2, burstsort2_superalphabet_brodnik)
  BUILD_SAMPLE_SORT(rantala_burstsort2, burstsort2_superalphabet_bagwell)
  BUILD_SAMPLE_SORT(rantala_burstsort2, burstsort2_superalphabet_vector_block)
  BUILD_SAMPLE_SORT(rantala_burstsort2, burstsort2_sampling_vector)
  BUILD_SAMPLE_SORT(rantala_burstsort2, burstsort2_sampling_brodnik)
  BUILD_SAMPLE_SORT(rantala_burstsort2, burstsort2_sampling_bagwell)
  BUILD_SAMPLE_SORT(rantala_burstsort2, burstsort2_sampling_vector_block)
  BUILD_SAMPLE_SORT(rantala_burstsort2, burstsort2_sampling_superalphabet_vector)
  BUILD_SAMPLE_SORT(rantala_burstsort2, burstsort2_sampling_superalphabet_brodnik)
  BUILD_SAMPLE_SORT(rantala_burstsort2, burstsort2_sampling_superalphabet_bagwell)
  BUILD_SAMPLE_SORT(rantala_burstsort2, burstsort2_sampling_superalphabet_vector_block)

  BUILD_SAMPLE_SORT(rantala_burstsort_mkq, burstsort_mkq_simpleburst_1)
  BUILD_SAMPLE_SORT(rantala_burstsort_mkq, burstsort_mkq_simpleburst_2)
  BUILD_SAMPLE_SORT(rantala_burstsort_mkq, burstsort_mkq_simpleburst_4)

/*******************************************************************************
 * Register funnelsort variants as local string sorter within our distributed
 * sample sort.
 ******************************************************************************/

  BUILD_SAMPLE_SORT(rantala, funnelsort_8way_bfs)
  BUILD_SAMPLE_SORT(rantala, funnelsort_16way_bfs)
  BUILD_SAMPLE_SORT(rantala, funnelsort_32way_bfs)
  BUILD_SAMPLE_SORT(rantala, funnelsort_64way_bfs)
  BUILD_SAMPLE_SORT(rantala, funnelsort_128way_bfs)
  BUILD_SAMPLE_SORT(rantala, funnelsort_8way_dfs)
  BUILD_SAMPLE_SORT(rantala, funnelsort_16way_dfs)
  BUILD_SAMPLE_SORT(rantala, funnelsort_32way_dfs)
  BUILD_SAMPLE_SORT(rantala, funnelsort_64way_dfs)
  BUILD_SAMPLE_SORT(rantala, funnelsort_128way_dfs)

/*******************************************************************************
 * Register mergesort variants as local string sorter within our distributed
 * sample sort.
 ******************************************************************************/

  BUILD_SAMPLE_SORT(rantala, mergesort_2way)
  BUILD_SAMPLE_SORT(rantala, mergesort_3way)
  BUILD_SAMPLE_SORT(rantala, mergesort_4way)
  BUILD_SAMPLE_SORT(rantala, mergesort_losertree_64way)
  BUILD_SAMPLE_SORT(rantala, mergesort_losertree_128way)
  BUILD_SAMPLE_SORT(rantala, mergesort_losertree_256way)
  BUILD_SAMPLE_SORT(rantala, mergesort_losertree_512way)
  BUILD_SAMPLE_SORT(rantala, mergesort_losertree_1024way)
  BUILD_SAMPLE_SORT(rantala, mergesort_2way_unstable)
  BUILD_SAMPLE_SORT(rantala, mergesort_3way_unstable)
  BUILD_SAMPLE_SORT(rantala, mergesort_4way_unstable)

} // namespace dsss::sample_sort

/******************************************************************************/
