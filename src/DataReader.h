//
// Created by Xuan Huang on 2022/4/11.
//

#ifndef BEAMSENSE_DATAREADER_H
#define BEAMSENSE_DATAREADER_H

#include <complex>
#include <string>

#include "conf.h"

class DataReader {
public:
    DataReader() = default;

    ~DataReader() = default;

    void readMatFromFile(std::string data_path, std::complex<double> ***mat, int NUM_SC, int NUM_ROW, int NUM_COL);

    void readAvgSFromFile(std::string data_path, double *avgS, int NUM_ANT);

    void readScidxFromFile(std::string data_path, int *sc_idx, int NUM_ANT);
};


#endif //BEAMSENSE_DATAREADER_H
