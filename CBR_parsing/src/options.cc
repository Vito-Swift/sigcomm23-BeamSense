
/***************************************************************************
 Copyright © 2021 Vito Wu. All rights reserved.
 **************************************************************************/

/**
 * File: src/options.cc
 * Author: Vito Wu[chenhaowu@link.cuhk.edu.cn]
 * Created At: 2021-11-03 13:38:55
 * desc:
 */

#include "options.hh"


namespace MTOOL_Utils {

    static void print_usage(char *prog_name) {
        printf("\n"
               "Usage: %s [OPTIONS]                        \n"
               "                                           \n"
               "                                           \n"
               "Options:                                   \n"
               "                                           \n"
               " --verbose [DEBUG_LEVEL]                   \n"
               "             set verbosity of the program  \n"
               "             - 0: silent                   \n"
               "             - 1: error                    \n"
               "             - 2: warn                     \n"
               "             - 3: info (default)           \n"
               "             - 4: debug (max verbosity)    \n"
               "                                           \n"
               " --mode [MODE]                             \n"
               "             set capture mode              \n"
               "             - 'pcap': use pcap library to capture on monitored device\n"
               "             - '8814au': use modified 8814au driver interface         \n"
               "                                           \n"
               " --iface [INTERFACE]                       \n"
               "             set wireless interface to use \n"
               "             e.g. 'wlan1', 'wlp2s0', etc.  \n"
               "                                           \n"
               " --filter [exp1,exp2,...,expn]                      \n"
               "       - legitimate expression type 1:              \n"
               "            set the type of packets to capture      \n"
               "            Supported Packet Types:                 \n"
               "             - 'NDPR': capture NDP report packet    \n"
               "             - 'CSI': capture CSI frame             \n"
               " --port [PORT]                             \n"
               "             set port to streaming capture \n"
               "             output                        \n"
               "                                           \n"
               " --no_gui                                  \n"
               "             disable QT GUI (default)      \n"
               "                                           \n"
               " --gui                                     \n"
               "             enable QT GUI                 \n",
               prog_name);
    }

    static inline void copy_opt(char **str, char *optarg) {
        if (nullptr == ((*str) = strndup(optarg, 1024))) {
            PRINTF_ERR("[!] invalid input file\n");
        }
    }

    void options_init(Options *opts) {
        opts->operation_mode = op_mode::unknown;
        opts->verbosity = debug_level::disable;
        opts->enable_gui = false;
        opts->o_port = 1110;
    }

    void options_free(Options *opts) {
        if (opts->iface)
            free(opts->iface);
    }

#define OP_VERBOSE  1
#define OP_MODE     2
#define OP_IFACE    3
#define OP_NO_GUI   4
#define OP_GUI      5
#define OP_PORT     6
#define OP_FILTER   7
#define OP_HOST     8

    static struct option MT_long_opts[] = {
            {"verbose", no_argument,       nullptr, OP_VERBOSE},
            {"mode",    required_argument, nullptr, OP_MODE},
            {"iface",   required_argument, nullptr, OP_IFACE},
            {"no_gui",  no_argument,       nullptr, OP_NO_GUI},
            {"gui",     no_argument,       nullptr, OP_GUI},
            {"port",    required_argument, nullptr, OP_PORT},
            {"filter",  required_argument, nullptr, OP_FILTER},
            {"host",    required_argument, nullptr, OP_HOST},
            {"help",  0,                   nullptr, 'h'},
            {nullptr, 0,                   nullptr, 0}
    };

    MTAPI options_parse(Options *opts, int argc, char **argv) {
        int c, opt_idx;
        if (argc == 1) {
            print_usage(argv[0]);
            exit(0);
        }
        while (-1 != (c = getopt_long(argc, argv, "h", MT_long_opts, &opt_idx))) {
            switch (c) {
                case 0:
                    /* If opt options set a flag, don't do anyghing */
                    if (MT_long_opts[opt_idx].flag == nullptr) {
                        PRINTF_STAMP("\toption %s: %s\n", MT_long_opts[opt_idx].name,
                                     optarg ? optarg : "null");
                    }
                    break;

                case 'h':
                    print_usage(argv[0]);
                    options_free(opts);
                    return MT_RET_FAIL;

                case OP_VERBOSE:
                    opts->verbosity = debug_level::debug;
                    break;

                case OP_MODE: {
                    char *str;
                    copy_opt(&str, optarg);
                    if (strcmp(str, "pcap") == 0) {
                        PRINTF_STAMP("option mode libpcap\n");
                        opts->operation_mode = op_mode::pcap;

                    } else if (strcmp(str, "rtl8814au") == 0) {
                        PRINTF_STAMP("option mode rtl8814au\n");
                        opts->operation_mode = op_mode::rtl8814au;

                    } else {
                        PRINTF_ERR_STAMP("option mode Unknown: %s\n", str);
                        opts->operation_mode = op_mode::unknown;

                    }
                    SFREE(str);
                    break;
                }

                case OP_IFACE: {
                    copy_opt(&opts->iface, optarg);
                    PRINTF_STAMP("\t\toption interface: %s\n", opts->iface);
                    break;
                }

                case OP_PORT: {
                    opts->o_port = strtoul(optarg, nullptr, 0);
                    PRINTF_STAMP("\t\toption port: %d\n", opts->o_port);
                    break;
                }

                case OP_NO_GUI: {
                    opts->enable_gui = false;
                    PRINTF_STAMP("\t\toption disable gui\n");
                    break;
                }

                case OP_GUI: {
                    opts->enable_gui = true;
                    PRINTF_STAMP("\t\toption enable gui\n");
                    break;
                }

                case OP_FILTER: {
                    copy_opt(&opts->i_filter, optarg);
                    PRINTF_STAMP("\t\traw filter options: %s\n", opts->i_filter);
                    break;
                }

                case OP_HOST: {
                    copy_opt(&opts->o_hostname, optarg);
                    PRINTF_STAMP("\t\thost ip: %s\n", opts->o_hostname);
                    break;
                }

                case '?':
                    /* getopt_long already printed an error message */
                    break;

                default:
                    EXIT_WITH_MSG("[!] unknown error, exit...\n");
            }
        }

        return MT_RET_SUCCESS;
    }
}
