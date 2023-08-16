//
// Created by Vito Wu on 13/11/2021.
//

#ifndef MIMO_TOOLS_FILTER_HH
#define MIMO_TOOLS_FILTER_HH

#include <regex>

#include "common.hh"
#include "dot11vhtndp_utils.hh"

namespace MTOOL_Utils {

    typedef struct {
        uint64_t RA;
        uint64_t TA;
        uint64_t DST;
        uint64_t SRC;
    } source_sink_rule_t;

    typedef struct {
        source_sink_rule_t ss_rule;
    } Rules;

    void parse_filter_rules(Rules *rules, char *user_input);

    bool check_filter_rules_aph(Rules *rules, dot11::dot11MAC_header* header);

}

#endif //MIMO_TOOLS_FILTER_HH
