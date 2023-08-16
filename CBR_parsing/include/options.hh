/***************************************************************************
 Copyright © 2021 Vito Wu. All rights reserved.
 **************************************************************************/

/**
 * File: include/options.hh
 * Author: Vito Wu[chenhaowu@link.cuhk.edu.cn]
 * Created At: 2021-11-03 15:32:02
 * desc:
 */

#ifndef MIMO_TOOLS_OPTIONS_H
#define MIMO_TOOLS_OPTIONS_H

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <getopt.h>
#include <cstring>
#include <ctime>
#include <string>

#include "common.hh"

namespace MTOOL_Utils {

    /* difference levels of debug verbosity */
    enum debug_level {
        disable = 0,    // no verbosity
        error,          // only print out spd_error
        warn,           // print out spd_error + spd_warn
        info,           // (default)
        debug           // max verbosity
    };

    /* operation mode*/
    enum op_mode {
        pcap = 0,       // (default) use pcap to capture trace on monitored interface    
        rtl8814au,      // TODO: use modified 8814au driver interface to capture trace
        unknown,
    };

    typedef struct {
        debug_level verbosity;      // log level options
        op_mode operation_mode;     // capture mode options
        char *iface;                // capture device
        char *o_hostname;           // output host ip address
        uint32_t o_port;            // output port
        bool enable_gui;            // whether to start QT gui
        char *i_filter;             // raw string of filters
    } Options;

    void options_init(Options *opts);

    void options_free(Options *opts);

    MTAPI options_parse(Options *opts, int argc, char **argv);

}

#endif // MIMO_TOOLS_OPTIONS_H
