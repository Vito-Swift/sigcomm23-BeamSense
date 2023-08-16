//
// Created by Vito Wu on 13/11/2021.
//

#include "mttool_interface.hh"

int main(int argc, char **argv) {
    MTOOL::Environment env;
    MTOOL::init_env(&env, argc, argv);
    MTOOL::mtool_entry(&env);
    return 0;
}
