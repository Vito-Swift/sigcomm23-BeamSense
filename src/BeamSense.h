//
// Created by Xuan Huang on 2022/4/27.
//

#ifndef BEAMSENSE_BEAMSENSE_H
#define BEAMSENSE_BEAMSENSE_H

#include <complex>
#include <vector>
#include <random>

#include "conf.h"

struct PathPara {
    double phi;
    double psi;
    double dis;
    double att;

    PathPara() {}

    PathPara(PathPara const &obj) {
        phi = obj.phi;
        psi = obj.psi;
        dis = obj.dis;
        att = obj.att;
    }

    bool operator< (PathPara const &obj) const {
        return phi < obj.phi;
    }

    double get_att() { return att; }
    double get_aoa() { return std::asin(phi); }
    double get_aod() { return std::asin(psi); }
    double get_dis() { return dis; }
};

class BeamSense {
public:
    BeamSense() = default;

    BeamSense(std::complex<double> ***UH,
              std::complex<double> ***V,
              double *avgS,
              int *sc_idx,
              int NUM_PATH,
              int NUM_SC,
              int NUM_TX,
              int NUM_RX);

    ~BeamSense();

    virtual void iter_search();

    void refinement();

    void update_domain(std::string domain_name, int path_id);

    double estimate(int path_id);

    void initialization_random();

    void gen_tx_steering_vector(double angle, std::complex<double> *vec, double lamb);
    void gen_rx_steering_vector(double angle, std::complex<double> *vec, double lamb);

    void update_matrix(std::complex<double> c, std::complex<double> *x, std::complex<double> *y,
                       std::complex<double> **z);

    double compare_matrix(std::complex<double> **x, int diag_flag);

    void mat_mul(std::complex<double> **x, std::complex<double> **y, std::complex<double> **z, int NUM_ROW,
                 int NUM_COL, int NUM_MID);
    void mat_add(std::complex<double> **x, std::complex<double> **y, int NUM_ROW, int NUM_COL, bool is_sub=false);

    std::complex<double> ***UH;
    std::complex<double> ***V;
    std::complex<double> ****path_S;
    std::complex<double> ***cur_S;
    double *avgS;
    int* sc_idx;

    std::vector<double> ang_space;
    std::vector<double> dis_space;
    std::vector<double> att_space;

    int num_path;
    PathPara *path_para;
    double score;

    int num_ini = 1; // number of initialization candidate for each path
    PathPara *ini_path_para;
    bool is_random_ini;
    std::random_device rd;
    std::uniform_int_distribution<int> ang_dist;
    std::uniform_int_distribution<int> att_dist;
    std::uniform_int_distribution<int> dis_dist;

    int NUM_SC = 234;
    double C_FREQ = 5.18e9;
    double ANT_SPACING = 0.027;
    int NUM_STS = 2;
    int NUM_TX;
    int NUM_RX;
    int step=10;
};


#endif //BEAMSENSE_BEAMSENSE_H
