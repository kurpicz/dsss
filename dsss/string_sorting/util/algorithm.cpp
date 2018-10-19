/*******************************************************************************
 * string_sorting/util/algorithm.cpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include "string_sorting/util/algorithm.hpp"
#include "util/string_set.hpp"

namespace dsss {

void sequential_sorter::run(dsss::string_set& strings) const {
  sorter_(strings.strings(), strings.size());
}

algorithm_category sequential_sorter::category() const {
  return algorithm_category::SEQUENTIAL;
}


void distributed_sorter::run(dsss::string_set& strings) const {
  sorter_(strings);
}

algorithm_category distributed_sorter::category() const {
  return algorithm_category::DISTRIBUTED;
}

void distributed_sorter::prepare_run() {
  
}

void distributed_sorter::concrete_run(dsss::string_set& /*strings*/) {

}

} // namespace dsss

/******************************************************************************/
