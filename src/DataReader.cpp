//
// Created by Xuan Huang on 2022/4/11.
//

#include "DataReader.h"

#include <fstream>
#include <iostream>

void DataReader::readMatFromFile(std::string data_path, std::complex<double> ***mat,
                                 int NUM_SC, int NUM_ROW, int NUM_COL) {
    FILE *fp = fopen(data_path.c_str(), "rb");
    double real, imag;
    for (int i = 0; i < NUM_SC; i++)
        for (int j = 0; j < NUM_ROW; j++)
            for (int k = 0; k < NUM_COL; k++) {
                fread(&real, sizeof(double), 1, fp);
                fread(&imag, sizeof(double), 1, fp);
                mat[i][j][k] = std::complex<double>(real, imag);
            }
    fclose(fp);
}

void DataReader::readAvgSFromFile(std::string data_path, double *avgS, int NUM_ANT) {
    FILE *fp = fopen(data_path.c_str(), "rb");
    double val;
    for (int i = 0; i < NUM_ANT; i++) {
        fread(&val, sizeof(double), 1, fp);
        avgS[i] = val;
    }
    fclose(fp);
}

void DataReader::readScidxFromFile(std::string data_path, int *sc_idx, int NUM_ANT) {
    FILE *fp = fopen(data_path.c_str(), "rb");
    int val;
    for (int i = 0; i < NUM_ANT; i++) {
        fread(&val, sizeof(int), 1, fp);
        sc_idx[i] = val;
    }
    fclose(fp);
}
