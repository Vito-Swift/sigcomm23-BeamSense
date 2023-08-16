//
// Created by Vito Wu on 13/11/2021.
//

#include "mttool_interface.hh"

using namespace MTOOL_Utils;

namespace MTOOL {

    void init_env(Environment *env, int argc, char **argv) {
        options_init(&env->options);
        options_parse(&env->options, argc, argv);
        parse_filter_rules(&env->rules, env->options.i_filter);
    }

    MTAPI mtool_entry(Environment *env) {
        mtool_prep(env);

        MTAPI ret = MT_RET_SUCCESS;

        if (env->options.operation_mode == op_mode::pcap) {
            PRINTF_STAMP("Enter operation mode: pcap eavesdropper\n");
            ret = mtool_pcap_entry(env);
        } else if (env->options.operation_mode == op_mode::rtl8814au) {
            PRINTF_STAMP("Enter operation mode: rtl8814au driver interface\n");
        } else {
            PRINTF_STAMP("Unrecognized operation mode, exit.\n");
            ret = MT_RET_FAIL;
        }

        return ret;
    }

    void free_env(Environment *env) {
        options_free(&env->options);
    }

}
