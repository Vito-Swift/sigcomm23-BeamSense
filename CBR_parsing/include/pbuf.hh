//
// Created by Vito Wu on 15/11/2021.
//

#ifndef MIMO_TOOLS_PBUF_HH
#define MIMO_TOOLS_PBUF_HH

#include <iostream>
#include <fstream>
#include <cstring>

#include "dot11vhtndp_utils.hh"
#include "common.hh"

namespace pbuf_interface {
    MTAPI write_ndp_report_to_udp(dot11::NDPFrame &frame,
                                  const std::vector<arma::cx_mat> &decoded_matrices,
                                  char *hostname,
                                  uint32_t port);
}

#endif //MIMO_TOOLS_PBUF_HH
