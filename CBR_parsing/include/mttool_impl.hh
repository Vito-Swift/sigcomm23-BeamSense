//
// Created by Vito Wu on 13/11/2021.
//

#ifndef MIMO_TOOLS_MTTOOL_IMPL_HH
#define MIMO_TOOLS_MTTOOL_IMPL_HH

#include <pcap/pcap.h>
#include <signal.h>

#include "common.hh"
#include "utils.h"
#include "options.hh"
#include "filter.hh"

namespace MTOOL {

    typedef struct {
        MTOOL_Utils::Options options;
        MTOOL_Utils::Rules rules;
        pcap_if_t *iface;
        pcap_t *descr;
    } Environment;

    MTAPI mtool_prep(Environment *env);

    MTAPI mtool_pcap_entry(Environment *env);

    MTAPI mtool_8814au_entry(Environment *env);
}

#endif //MIMO_TOOLS_MTTOOL_IMPL_HH
