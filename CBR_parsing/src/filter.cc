//
// Created by Vito Wu on 13/11/2021.
//

#include "filter.hh"

using namespace std;

namespace MTOOL_Utils {

    static const regex equal_exp(R"(\s*(\w+)=([\w\:]+)\s*)");

    inline vector<string> split_string_by_comma(const string &str) {
        stringstream ss(str);
        vector<string> result_str;
        while (ss.good()) {
            string substr;
            getline(ss, substr, ',');
            result_str.push_back(substr);
        }
        return result_str;
    }

    inline MTAPI convert_mac_string_to_uint64(const string &str, uint64_t *res) {
        unsigned char mac_bytes[6];
        int last = -1;
        int rc = sscanf(str.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx%n",
                        mac_bytes + 0, mac_bytes + 1, mac_bytes + 2, mac_bytes + 3, mac_bytes + 4, mac_bytes + 5,
                        &last);
        if (rc != 6 || str.size() != last) {
            PRINTF_ERR_STAMP("invalid mac address format %s\n", str.c_str());
            return MT_RET_FAIL;
        }

        *res = uint64_t(mac_bytes[0]) << 40 |
               uint64_t(mac_bytes[1]) << 32 |
               (uint32_t(mac_bytes[2]) << 24 |
                uint32_t(mac_bytes[3]) << 16 |
                uint32_t(mac_bytes[4]) << 8 |
                uint32_t(mac_bytes[5]));
        return MT_RET_SUCCESS;
    }

    inline void init_rules(Rules *rules) {
        rules->ss_rule.RA = 0;
        rules->ss_rule.TA = 0;
        rules->ss_rule.DST = 0;
        rules->ss_rule.SRC = 0;
    }

    void parse_filter_rules(Rules *rules, char *user_input) {
        init_rules(rules);
        PRINTF_STAMP("Parse filter rules:\n");
        vector<string> rule_str = split_string_by_comma(string(user_input));
        PRINTF_STAMP("\t\tretrieved %d filter expressions\n", rule_str.size());

        if (rule_str.empty())
            return;

        for (auto &r: rule_str) {
            std::smatch sm;

            /* check equal expression */
            if (regex_match(r, sm, equal_exp)) {
                auto lhs = sm[1];
                auto rhs = sm[2];

                uint64_t rhs_literal;
                if (convert_mac_string_to_uint64(rhs.str(), &rhs_literal) == MT_RET_FAIL) {
                    PRINTF_STAMP("\t\t- %s\t[UNKNOWN]\n", r.c_str());
                    continue;
                }

                if (strcmp(lhs.str().c_str(), "RA") == 0) {
                    rules->ss_rule.RA = rhs_literal;
                } else if (strcmp(lhs.str().c_str(), "TA") == 0) {
                    rules->ss_rule.TA = rhs_literal;
                } else if (strcmp(lhs.str().c_str(), "DST") == 0) {
                    rules->ss_rule.DST = rhs_literal;
                } else if (strcmp(lhs.str().c_str(), "SRC") == 0) {
                    rules->ss_rule.SRC = rhs_literal;
                } else {
                    PRINTF_STAMP("\t\t- %s\t[UNKNOWN]\n", r.c_str());
                    continue;
                }

                PRINTF_STAMP("\t\t- %s\t[EQUAL_EXP]\n", r.c_str());
            } else {
                PRINTF_STAMP("\t\t- %s\t[UNKNOWN]\n", r.c_str());
            }
        }
    }

    inline bool validate_source_sink_rules(const source_sink_rule_t &ss_rule, dot11::dot11MAC_header *header) {
        if (ss_rule.DST != 0 && ss_rule.DST != header->DST)
            return false;
        if (ss_rule.RA != 0 && ss_rule.RA != header->RA)
            return false;
        if (ss_rule.TA != 0 && ss_rule.TA != header->TA)
            return false;
        return true;
    }

    bool check_filter_rules_aph(Rules *rules, dot11::dot11MAC_header *header) {
        bool ret = true;
        ret = validate_source_sink_rules(rules->ss_rule, header);

        return ret;
    }
}
