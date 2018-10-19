/*******************************************************************************
 * string_sorting/sequential/sample_sort.cpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#pragma once

#include <cstdint>

#include "util/string.hpp"

namespace bingmann_sample_sortBSC_original {

extern void bingmann_sample_sortBSC_original(dsss::string* strings,
  std::size_t n);
extern void bingmann_sample_sortBSCA_original(dsss::string* strings,
  std::size_t n);
extern void bingmann_sample_sortBSCE_original(dsss::string* strings,
  std::size_t n);
extern void bingmann_sample_sortBSCEA_original(dsss::string* strings,
  std::size_t n);
  
} // namespace bingmann_sample_sortBSC_original

namespace bingmann_sample_sort {

extern void bingmann_sample_sortBSC(dsss::string* strings, std::size_t n);
extern void bingmann_sample_sortBTC(dsss::string* strings, std::size_t n);
extern void bingmann_sample_sortBTCA(dsss::string* strings, std::size_t n);
extern void bingmann_sample_sortBTCU(dsss::string* strings, std::size_t n);
extern void bingmann_sample_sortBTCUI(dsss::string* strings, std::size_t n);
extern void bingmann_sample_sortBTCT(dsss::string* strings, std::size_t n);
extern void bingmann_sample_sortBTCTU(dsss::string* strings, std::size_t n);
extern void bingmann_sample_sortBTCTUI(dsss::string* strings, std::size_t n);
extern void bingmann_sample_sortBTCE(dsss::string* strings, std::size_t n);
extern void bingmann_sample_sortBTCEA(dsss::string* strings, std::size_t n);
extern void bingmann_sample_sortBTCEU(dsss::string* strings, std::size_t n);
extern void bingmann_sample_sortBTCEV(dsss::string* strings, std::size_t n);

} // namespace bingmann_sample_sort

/******************************************************************************/
