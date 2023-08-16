//
// Created by Xuan Huang on 2022/4/27.
//

#include "BeamSense.h"

#include <cmath>
#include <ctime>
#include <iostream>
#include <queue>
#include <chrono>
#include <cstring>

BeamSense::BeamSense(std::complex<double> ***UH,
                     std::complex<double> ***V,
                     double *avgS,
                     int *sc_idx,
                     int NUM_PATH,
                     int NUM_SC,
                     int NUM_TX,
                     int NUM_RX) : BeamSense() {
    this->NUM_SC = NUM_SC;
    this->NUM_TX = NUM_TX;
    this->NUM_RX = NUM_RX;
    this->NUM_STS = std::min(NUM_TX, NUM_RX);
    this->UH = UH;
    this->V = V;
    this->avgS = avgS;
    this->sc_idx = sc_idx;
    this->path_para = (PathPara *) malloc(NUM_PATH * sizeof(PathPara));
    if (NUM_SC == 56) {
        C_FREQ = 2.462e9;
        ANT_SPACING = 0.054;
    }

/*  search space    */
    num_path = NUM_PATH;
    //    angle space
    for (int i = 0; i <= 200; i++) {
        ang_space.push_back(-1 + i * 0.01);
    }
    //    distance space
    for (int i = 0; i <= 200; i++) {
        dis_space.push_back(i * 0.05);
    }
    //    attenuation space
    for (int i = 1; i <= 200; i++) {
        att_space.push_back(i * 0.007);
    }
/*  search space    */

    /*  initialization method   */
    is_random_ini = true;
    num_ini = 1;

    ini_path_para = (PathPara *) malloc(num_ini * sizeof(PathPara));
    ang_dist = std::uniform_int_distribution<int>(0, (int)ang_space.size());
    att_dist = std::uniform_int_distribution<int>(0, (int)att_space.size());
    dis_dist = std::uniform_int_distribution<int>(0, (int)dis_space.size());
/*  initialization method   */

    path_S = (std::complex<double> ****) malloc(num_path * sizeof(std::complex<double> ***));
    for (int i = 0; i < num_path; i ++) {
        path_S[i] = (std::complex<double> ***) malloc(NUM_SC * sizeof(std::complex<double> **));
        for (int j = 0; j < NUM_SC; j ++) {
            path_S[i][j] = (std::complex<double> **) malloc(NUM_STS * sizeof(std::complex<double> *));
            for (int k = 0; k < NUM_STS; k ++) {
                path_S[i][j][k] = (std::complex<double> *) malloc(NUM_STS * sizeof(std::complex<double>));
                std::fill(path_S[i][j][k], path_S[i][j][k]+NUM_STS, 0);
                memset(path_S[i][j][k], 0, sizeof(std::complex<double>) * NUM_STS);
            }
        }
    }
    cur_S = (std::complex<double> ***) malloc(NUM_SC * sizeof(std::complex<double> **));
    for (int i = 0; i < NUM_SC; i ++) {
        cur_S[i] = (std::complex<double> **) malloc(NUM_STS * sizeof(std::complex<double> *));
        for (int j = 0; j < NUM_STS; j ++) {
            cur_S[i][j]= (std::complex<double> *) malloc(NUM_STS * sizeof(std::complex<double>));
            memset(cur_S[i][j], 0, sizeof(std::complex<double>) * NUM_STS);
        }
    }

    for (int i = 0; i < num_ini; i++) {
        ini_path_para[i].phi = 0;
        ini_path_para[i].psi = 0;
        ini_path_para[i].att = 0;
        ini_path_para[i].dis = 0;
    }

    for (int i = 0; i < num_path; i++) {
        path_para[i].phi = 0;
        path_para[i].psi = 0;
        path_para[i].att = 0;
        path_para[i].dis = 0;
    }
}

void BeamSense::iter_search() {
    double last_error;
    double eps = 1e-3;
    for (int p = 0; p < num_path; p++) {
        double min_score = 1e9;
        PathPara best_candidate;
        initialization_random();
        auto start = std::chrono::system_clock::now();
        for (int i = 0; i < num_ini; i ++) {
            path_para[p] = ini_path_para[i];
            last_error = 1e9;
            while (true) {
                update_domain("phi", p);
                update_domain("psi", p);
                update_domain("dis", p);
                update_domain("att", p);
                if (fabs(last_error - score) < eps) {
                    break;
                }
                last_error = score;
            }
            if (score < min_score) {
                best_candidate = path_para[p];
                min_score = score;
            }
        }
        path_para[p] = best_candidate;
        auto end = std::chrono::system_clock::now();
//        std::cout << "[INFO] initial iter score: " << min_score << ", time: " << (end-start).count()/1e6 << std::endl;
    }
    refinement();
}

void BeamSense::refinement() {
    double last_error = 1e9;
    double eps = 1e-3;
    while (true) {
        for (int p = 0; p < num_path; p++) {
            double last_score = score;
            while (true) {
                update_domain("phi", p);
                update_domain("psi", p);
                update_domain("dis", p);
                update_domain("att", p);
                if (fabs(last_score-score) < eps) {
                    break;
                }
                last_score = score;
            }
        }
        if (fabs(last_error-score) < eps) {
            break;
        }
        last_error = score;
    }
}

void BeamSense::initialization_random() {
    for (int i = 0; i < num_ini; i ++) {
        ini_path_para[i].phi = ang_space[ang_dist(rd)];
        ini_path_para[i].psi = ang_space[ang_dist(rd)];
        ini_path_para[i].att = att_space[att_dist(rd)];
        ini_path_para[i].dis = dis_space[dis_dist(rd)];
    }
}

void BeamSense::update_domain(std::string domain_name, int path_id) {
    std::vector<double> *space;
    double *para;
    if (domain_name == "phi") {
        space = &ang_space;
        para = &(path_para[path_id].phi);
    } else if (domain_name == "psi") {
        space = &ang_space;
        para = &(path_para[path_id].psi);
    } else if (domain_name == "dis") {
        space = &dis_space;
        para = &(path_para[path_id].dis);
    } else {
        space = &att_space;
        para = &(path_para[path_id].att);
    }

    double min_error = 1e9;
    double cur_error;
    double best_val;
    for (auto val : *space) {
        *para = val;
        cur_error = estimate(path_id);
        if (cur_error < min_error) {
            min_error = cur_error;
            best_val = val;
        }
    }
    *para = best_val;
    estimate(path_id);
    score = min_error;
}


double BeamSense::estimate(int path_id) {
    auto tx_vec = (std::complex<double> *) malloc(NUM_TX * sizeof(std::complex<double>));
    auto rx_vec = (std::complex<double> *) malloc(NUM_RX * sizeof(std::complex<double>));
    std::complex<double> c;
    auto H = (std::complex<double> **) malloc(NUM_RX * sizeof(std::complex<double> *));
    auto tmp = (std::complex<double> **) malloc(NUM_STS * sizeof(std::complex<double> *));
    for (int i = 0; i < NUM_RX; i++) {
        H[i] = (std::complex<double> *) malloc(NUM_TX * sizeof(std::complex<double>));
    }
    for (int i = 0; i < NUM_STS; i++) {
        tmp[i] = (std::complex<double> *) malloc(NUM_TX * sizeof(std::complex<double>));
    }

    double lamb, error = 0.0;
    int cnt = 0;
    for (int i = 0; i < NUM_SC; i += step, cnt++) {
        lamb = C / (C_FREQ + sc_idx[i] * SC_WIDTH);
        mat_add(cur_S[i], path_S[path_id][i], NUM_STS, NUM_STS, true);
        gen_rx_steering_vector(path_para[path_id].phi, rx_vec, lamb);
        gen_tx_steering_vector(path_para[path_id].psi, tx_vec, lamb);
        c = std::polar(path_para[path_id].att, -2 * M_PI * path_para[path_id].dis / lamb);
        update_matrix(c, rx_vec, tx_vec, (std::complex<double> **) H);
        mat_mul(UH[i], H, tmp, NUM_STS, NUM_TX, NUM_RX);
        mat_mul(tmp, V[i], path_S[path_id][i], NUM_STS, NUM_STS, NUM_TX);
        mat_add(cur_S[i], path_S[path_id][i], NUM_STS, NUM_STS);
        error += compare_matrix(cur_S[i], 3);
    }
    free(tx_vec);
    free(rx_vec);
    for (int i = 0; i < NUM_RX; i++) {
        free(H[i]);
    }
    for (int i = 0; i < NUM_STS; i++) {
        free(tmp[i]);
    }
    free(H);
    free(tmp);
    return error / cnt;
}

void BeamSense::gen_tx_steering_vector(double angle, std::complex<double> *vec, double lamb) {
    for (int i = 0; i < NUM_TX; i++) {
        vec[i] = std::polar(1.0, -ANT_SPACING * 2 * M_PI * angle / lamb * i);
    }
}

void BeamSense::gen_rx_steering_vector(double angle, std::complex<double> *vec, double lamb) {
    for (int i = 0; i < NUM_RX; i++) {
        vec[i] = std::polar(1.0, -ANT_SPACING * 2 * M_PI * angle / lamb * i);
    }
}

void BeamSense::update_matrix(std::complex<double> c, std::complex<double> *x, std::complex<double> *y,
                              std::complex<double> **z) {
    for (int i = 0; i < NUM_RX; i++) {
        for (int j = 0; j < NUM_TX; j++) {
            z[i][j] = c * x[i] * std::conj(y[j]);
        }
    }
}

double BeamSense::compare_matrix(std::complex<double> **x, int diag_flag) {
    double diff = 0.0;
    for (int i = 0; i < NUM_STS; i++) {
        for (int j = 0; j < NUM_STS; j++) {
            if (i == j) {
                if (diag_flag & 0x1)
                    diff += std::norm(std::abs(x[i][j]) - avgS[i]);
            } else {
                if (diag_flag & 0x2)
                    diff += std::norm(x[i][j]);
            }
        }
    }
    return std::sqrt(diff);
}

void BeamSense::mat_mul(std::complex<double> **x, std::complex<double> **y,
                        std::complex<double> **z,
                        int NUM_ROW,
                        int NUM_COL,
                        int NUM_MID) {
    for (int i = 0; i < NUM_ROW; i++) {
        for (int j = 0; j < NUM_COL; j++) {
            z[i][j] = 0;
            for (int k = 0; k < NUM_MID; k++) {
                z[i][j] += x[i][k] * y[k][j];
            }
        }
    }
}

void BeamSense::mat_add(std::complex<double> **x, std::complex<double> **y,
                        int NUM_ROW, int NUM_COL, bool is_sub) {
    for (int i = 0; i < NUM_ROW; i ++) {
        for (int j = 0; j < NUM_COL; j ++) {
            if (!is_sub)
                x[i][j] += y[i][j];
            else
                x[i][j] -= y[i][j];
        }
    }
}

BeamSense::~BeamSense() {
    free(this->path_para);
    free(this->ini_path_para);
    for (int i = 0; i < num_path; i ++) {
        for (int j = 0; j < NUM_SC; j ++) {
            for (int k = 0; k < NUM_STS; k ++) {
                free(path_S[i][j][k]);
            }
            free(path_S[i][j]);
        }
        free(path_S[i]);
    }
    free(path_S);
    for (int i = 0; i < NUM_SC; i ++) {
        for (int j = 0; j < NUM_STS; j ++) {
            free(cur_S[i][j]);
        }
        free(cur_S[i]);
    }
    free(cur_S);
}
