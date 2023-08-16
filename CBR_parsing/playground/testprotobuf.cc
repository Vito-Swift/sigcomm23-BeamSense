//
// Created by Vito Wu on 13/11/2021.
//

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include "proto/ndpreport.pb.h"


int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << "[R/W] [STORE_FILE]" << std::endl;
        return -1;
    }

    if (strcmp(argv[1], "1") == 0) {
        // write mode
        MIMO_Toolkit::NDPReport ndpReport;

    } else if (strcmp(argv[1], "2") == 0) {
        // read mode

    } else {
        std::cerr << "Usage: " << argv[0] << "[R/W] [STORE_FILE]" << std::endl;
        return -1;
    }

    return 0;
}
