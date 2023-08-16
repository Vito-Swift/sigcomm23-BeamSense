//
// Created by Vito Wu on 15/11/2021.
//

#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <string>
#include "proto/ndpreport.pb.h"
#include "dot11vhtndp_utils.hh"
#include "pbuf.hh"

namespace pbuf_interface {

    MTAPI write_ndp_report_to_udp(dot11::NDPFrame &frame,
                                  const std::vector<arma::cx_mat> &decoded_matrices,
                                  char *hostname,
                                  uint32_t port) {
        int num_bytes;
        int sock_fd;
        sockaddr_in servaddr;
        bzero(&servaddr, sizeof(servaddr));

        /* fill in google protocal buffer */
        MIMO_Toolkit::NDPReport to;
        timeval tv;
        gettimeofday(&tv, nullptr);
        to.mutable_timestamp()->set_seconds(tv.tv_sec);
        to.mutable_timestamp()->set_nanos(tv.tv_usec * 1000);
        to.mutable_mac()->set_ra(frame.header->dot11MacHeader.RA);
        to.mutable_mac()->set_ta(frame.header->dot11MacHeader.TA);
        to.mutable_mac()->set_src(frame.header->dot11MacHeader.TA);
        to.mutable_mac()->set_dst(frame.header->dot11MacHeader.DST);
        to.mutable_mimocontrol()->set_nr(frame.dot11vhtmimoCtrlFrame.nr);
        to.mutable_mimocontrol()->set_nc(frame.dot11vhtmimoCtrlFrame.nc);
        to.mutable_mimocontrol()->set_ng(frame.dot11vhtmimoCtrlFrame.ng);

        // fill compressed beamforming matrix
        for (int i = 0; i < frame.dot11vhtmimoCtrlFrame.nscarrier; i++) {
            MIMO_Toolkit::NDPReport_V *V = to.add_steering_matrices();
            V->set_col_num(decoded_matrices[i].n_cols);
            V->set_row_num(decoded_matrices[i].n_rows);
            for (int j = 0; j < V->col_num(); j++) {
                for (int k = 0; k < V->col_num(); k++) {
                    MIMO_Toolkit::NDPReport_compl *c = V->add_content();
                    c->set_real(decoded_matrices[i](j, k).real());
                    c->set_imag(decoded_matrices[i](j, k).imag());
                }
            }
        }

        // fill ASNR
        for (int i = 0; i < frame.dot11vhtmimoCtrlFrame.ns; i++) {
            to.add_asnr(frame.dot11vhtbfReportFrame.ASNR[i]);
        }

        sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock_fd == -1) {
            PRINTF_ERR_STAMP("Could not create socket\n");
            return MT_RET_FAIL;
        }
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);
        if (inet_pton(AF_INET, hostname, &servaddr.sin_addr) <= 0) {
            PRINTF_ERR_STAMP("Invalid host address %s\n", hostname);
            return MT_RET_FAIL;
        }

        size_t packet_size = to.ByteSizeLong();
        char *buff = (char *) malloc(packet_size);
        to.SerializeToArray(buff, packet_size);

        if (sendto(sock_fd, buff, packet_size, 0,
                   (sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
            PRINTF_ERR_STAMP("Send failed");
            free(buff);
            close(sock_fd);
            return MT_RET_SUCCESS;
        }

        free(buff);
        close(sock_fd);

        PRINTF_STAMP("Sent one packet: %s : %d\n", hostname, port);
        return MT_RET_SUCCESS;
    }

}
