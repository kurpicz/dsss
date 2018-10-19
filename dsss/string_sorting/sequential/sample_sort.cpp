/*******************************************************************************
 * string_sorting/sequential/sample_sort.cpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include "string_sorting/sequential/sample_sort.hpp"
#include "string_sorting/util/algorithm.hpp"

namespace bingmann_sample_sortBSC_original {

REGISTER_SEQUENTIAL_SORTER(bingmann_sample_sortBSC_original,
  "sequential/sample_sortBSC_original",
  "Sample_sortBSC_original (binary search, bkt cache)")
REGISTER_SEQUENTIAL_SORTER(bingmann_sample_sortBSCA_original,
  "sequential/sample_sortBSCA_original",
  "Sample_sortBSCA_original (binary search, assembler CMOV, bkt cache)")
REGISTER_SEQUENTIAL_SORTER(bingmann_sample_sortBSCE_original,
  "sequential/sample_sortBSCE_original",
  "Sample_sortBSCE_original (binary search equal, bkt cache)")
REGISTER_SEQUENTIAL_SORTER(bingmann_sample_sortBSCEA_original,
  "sequential/sample_sortBSCEA_original",
  "Sample_sortBSCEA_original (binary search equal, assembler CMOV, bkt cache)")

} // namespace bingmann_sample_sortBSC_original

namespace bingmann_sample_sort {

REGISTER_SEQUENTIAL_SORTER(bingmann_sample_sortBSC,
  "sequential/sample_sortBSC",
  "sample_sortBSC (binary search, bkt cache)")
REGISTER_SEQUENTIAL_SORTER(bingmann_sample_sortBTC,
  "sequential/sample_sortBTC",
  "sample_sortBTC (binary tree, bkt cache)")
REGISTER_SEQUENTIAL_SORTER(bingmann_sample_sortBTCA,
  "sequential/sample_sortBTCA",
  "sample_sortBTCA (binary tree, asm CMOV, bkt cache)")
REGISTER_SEQUENTIAL_SORTER(bingmann_sample_sortBTCU,
  "sequential/sample_sortBTCU",
  "sample_sortBTCU (binary tree, bkt cache)")
REGISTER_SEQUENTIAL_SORTER(bingmann_sample_sortBTCUI,
  "sequential/sample_sortBTCUI",
  "sample_sortBTCUI (binary tree, bkt cache)")
REGISTER_SEQUENTIAL_SORTER(bingmann_sample_sortBTCT,
  "sequential/sample_sortBTCT",
  "sample_sortBTCT (binary tree, bkt cache, tree calc)")
REGISTER_SEQUENTIAL_SORTER(bingmann_sample_sortBTCTU,
  "sequential/sample_sortBTCTU",
  "sample_sortBTCTU (binary tree, bkt cache, tree calc)")
REGISTER_SEQUENTIAL_SORTER(bingmann_sample_sortBTCTUI,
  "sequential/sample_sortBTCTUI",
  "sample_sortBTCTUI (binary tree, bkt cache, tree calc)")
REGISTER_SEQUENTIAL_SORTER(bingmann_sample_sortBTCE,
  "sequential/sample_sortBTCE",
  "sample_sortBTCE (binary tree equal, bkt cache)")
REGISTER_SEQUENTIAL_SORTER(bingmann_sample_sortBTCEA,
  "sequential/sample_sortBTCEA",
  "sample_sortBTCEA (binary tree equal, asm CMOV, bkt cache)")
REGISTER_SEQUENTIAL_SORTER(bingmann_sample_sortBTCEU,
  "sequential/sample_sortBTCEU",
  "sample_sortBTCEU (binary tree equal unroll, asm CMOV, bkt cache)")
REGISTER_SEQUENTIAL_SORTER(bingmann_sample_sortBTCEV,
  "sequential/sample_sortBTCEV",
  "sample_sortBTCEV (binary tree equal unroll, asm CMOV, bkt cache)")

} // namespace bingmann_sample_sort

/******************************************************************************/
