//
// Created by Vito Wu on 14/11/2021.
//

#include <iostream>
#include <string>
#include <regex>


int main() {
    std::string test_str("RA=w0:23:a0:s9,SRC=134, asd=1203, packet type management");
    std::stringstream ss(test_str);
    std::vector<std::string> rule_str;

    while (ss.good()) {
        std::string substr;
        getline(ss, substr, ',');
        rule_str.push_back(substr);
    }

    for (auto &r : rule_str)
        std::cout << r << std::endl;

    // check equal expression
    std::regex equal_exp(R"(\s*(\w+)=([\w\:]+)\s*)");
    for (auto &r : rule_str) {
        std::smatch sm;
        if (std::regex_match(r, sm, equal_exp)) {
            for (auto &m: sm) {
                std::cout << "[" << m << "]";
            }
            std::cout << std::endl;
        }
    }

    return 0;
}
