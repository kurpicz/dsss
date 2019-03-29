/*******************************************************************************
 * mpi/broadcast.hpp
 *
 * Copyright (C) 2018 Florian Kurpicz <florian.kurpicz@tu-dortmund.de>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file. 
 ******************************************************************************/

#pragma once

namespace dsss::mpi {

template <typename DataType>
inline DataType broadcast(DataType send_data, int32_t const root,
                          environment env =  environment()) {

  data_type_mapper<DataType> dtm;
  DataType result = send_data;
  MPI_Bcast(&result,
            1,
            dtm.get_mpi_type(),
            root,
            env.communicator());
  return result;
}

inline std::string broadcast(std::string& send_data, int32_t const root,
                             environment env = environment()) {
 
  auto result = send_data;
  int32_t size = broadcast(result.size(), root);

  data_type_mapper<char> dtm;
  std::vector<char> buffer(size);
  MPI_Bcast(result.data(),
            size,
            dtm.get_mpi_type(),
            root,
            env.communicator());
  result = std::string(buffer.data(), size);
  return result;
}

} // namespace dsss::mpi

/******************************************************************************/
