//
// Created by Vito Wu on 13/11/2021.
//

#ifndef MIMO_TOOLS_MTTOOL_INTERFACE_HH
#define MIMO_TOOLS_MTTOOL_INTERFACE_HH

#include <iostream>
#include <pcap.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <signal.h>

#include "common.hh"
#include "options.hh"
#include "filter.hh"
#include "mttool_impl.hh"

namespace MTOOL {

    void init_env(Environment *env, int argc, char **argv);

    MTAPI mtool_entry(Environment *env);

    void free_env(Environment *env);
}

#endif //MIMO_TOOLS_MTTOOL_INTERFACE_HH
