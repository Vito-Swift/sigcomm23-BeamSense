/*
 * Executable used to interfacing ACA optimization and calls from other programs
 */

#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include "DataReader.h"
#include "BeamSense.h"


int main(int argc, char **argv) {
    // argv[1]: V_path
    // argv[2]: S_path
    // argv[3]: UH_path
    const std::string V_path = std::string(argv[1]);
    const std::string S_path = std::string(argv[2]);
    const std::string UH_path = std::string(argv[3]);
    const std::string sc_path = std::string(argv[4]);
    int NUM_PATH = std::stoi(argv[5]);
    int NUM_SC = std::stoi(argv[6]);
    int NUM_TX = std::stoi(argv[7]);
    int NUM_RX = std::stoi(argv[8]);
    int NUM_STS = std::min(NUM_TX, NUM_RX);

    // read data from file
    DataReader dataReader;
    auto ***V = (std::complex<double> ***) malloc(NUM_SC * sizeof(std::complex<double> **));
    auto ***UH = (std::complex<double> ***) malloc(NUM_SC * sizeof(std::complex<double> **));
    for (int i = 0; i < NUM_SC; i++) {
        V[i] = (std::complex<double> **) malloc(NUM_TX * sizeof(std::complex<double> *));
        UH[i] = (std::complex<double> **) malloc(NUM_STS * sizeof(std::complex<double> *));
        for (int j = 0; j < NUM_TX; j++) {
            V[i][j] = (std::complex<double> *) malloc(NUM_STS * sizeof(std::complex<double>));
        }
        for (int j = 0; j < NUM_STS; j++) {
            UH[i][j] = (std::complex<double> *) malloc(NUM_RX * sizeof(std::complex<double>));
        }
    }

    auto *S = (double *) malloc(NUM_STS * sizeof(double));
    auto *sc_idx = (int*) malloc(NUM_SC * sizeof(int));

    dataReader.readMatFromFile(V_path, V, NUM_SC, NUM_TX, NUM_STS);
    dataReader.readMatFromFile(UH_path, UH, NUM_SC, NUM_STS, NUM_RX);
    dataReader.readAvgSFromFile(S_path, S, NUM_STS);
    dataReader.readScidxFromFile(sc_path, sc_idx, NUM_SC);


    // estimation
    std::vector<std::vector<double>> path_output;
    BeamSense beamSense((std::complex<double> ***) UH,
                        (std::complex<double> ***) V,
                        S, sc_idx, NUM_PATH, NUM_SC, NUM_TX, NUM_RX);

    auto start = std::chrono::system_clock::now();
    beamSense.iter_search();
    auto end = std::chrono::system_clock::now();

    for (int p = 0; p < NUM_PATH; p++) {
        path_output.emplace_back();
        path_output.back().push_back(p); // path_idx
        path_output.back().push_back(beamSense.path_para[p].att);
        path_output.back().push_back(beamSense.path_para[p].phi); // aoa
        path_output.back().push_back(beamSense.path_para[p].psi); // aod
        path_output.back().push_back(beamSense.path_para[p].dis); // dis
    }

    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count() / (long double)1e6;
    for (auto &po: path_output) {
        std::cout << po[0] << "," << po[1] << "," << po[2] << "," << po[3] << ","  << po[4] << "\n";
    }

    for (int i = 0; i < NUM_SC; i++) {
        for (int j = 0; j < NUM_TX; j++) {
            free(V[i][j]);
        }
        for (int j = 0; j < NUM_STS; j++) {
            free(UH[i][j]);
        }
        free(V[i]);
        free(UH[i]);
    }
    free(V);
    free(UH);
    free(S);
    free(sc_idx);
    return EXIT_SUCCESS;
}