//
// Created by Vito Wu on 4/11/2021.
//

#ifndef MIMO_TOOLS_DOT11VHTNDP_UTILS_HH
#define MIMO_TOOLS_DOT11VHTNDP_UTILS_HH

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <pcap.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <ctime>
#include <iomanip>
#include <string>
#include <sstream>
#include <armadillo>

#include "common.hh"

namespace dot11 {

#define dot11_PACKET_TYPE_MGMT                  (0x0)
#define dot11_PACKET_TYPE_CTRL                  (0x1)
#define dot11_PACKET_TYPE_DATA                  (0x2)
#define dot11_SUBTYPE_ASSO_REQ                  (0x0)
#define dot11_SUBTYPE_ASSO_RES                  (0x1)
#define dot11_SUBTYPE_REASSO_REQ                (0x2)
#define dot11_SUBTYPE_REASSO_RES                (0x3)
#define dot11_SUBTYPE_PROBE_REQ                 (0x4)
#define dot11_SUBTYPE_PROBE_RES                 (0x5)
#define dot11_SUBTYPE_BEACON                    (0x8)
#define dot11_SUBTYPE_ATIM                      (0x9)
#define dot11_SUBTYPE_DISASSO                   (0xa)
#define dot11_SUBTYPE_AUTH                      (0xb)
#define dot11_SUBTYPE_DEAUTH                    (0xc)
#define dot11_SUBTYPE_ACTION_NO_ACK             (0xe)
#define dot11_ACTION_NDP                        (0x0)
#define dot11_VHT_NDP_FEEDBACK_SU               (0x0)
#define dot11_VHT_NDP_FEEDBACK_MU               (0x1)
#define dot11_VHT_NDP_ANGLE_EMP                 (0x0)
#define dot11_VHT_NDP_ANGLE_PSI                 (0x1)
#define dot11_VHT_NDP_ANGLE_PHI                 (0x2)

    /**
     * angle_representation_table: (from 802.11-2013 table 8-53d)
     *    This table stores the
     *      0 stands for empty
     *      1 stands for psi
     *      2 stands for phi
     */
    const char angle_representation_table[9][12] = {
            {2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},       // 2 x 1
            {2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},       // 2 x 2
            {2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},       // 3 x 1
            {2, 2, 1, 1, 2, 1, 0, 0, 0, 0, 0, 0},       // 3 x 2
            {2, 2, 1, 1, 2, 1, 0, 0, 0, 0, 0, 0},       // 3 x 3
            {2, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0},       // 4 x 1
            {2, 2, 2, 1, 1, 1, 2, 2, 1, 1, 0, 0},       // 4 x 2
            {2, 2, 2, 1, 1, 1, 2, 2, 1, 1, 2, 1},       // 4 x 3
            {2, 2, 2, 1, 1, 1, 2, 2, 1, 1, 2, 1},       // 4 x 4
    };

    struct radiotap_header {
        uint8_t it_rev;                                 // Revision: version fo radiotap
        uint8_t it_pad;                                 // Padding: 0 - Aligns the fields onto natural word boundaries
        uint16_t it_len;                                // Length: 26 - entire length of RadioTap Header
    };

    struct dot11MAC_header {
        uint8_t type;                                   // type of ths packet (mgmt, ctrl, data)
        uint8_t subtype;                                // subtype of this frame (beacon, association, action no ack, etc.)
        uint8_t version;
        uint64_t RA;                                    // mac address to receiver
        uint64_t TA;                                    // mac address of transmitter
        uint64_t DST;                                   // mac address of destination
        uint64_t BSS;                                   // id of this basic service set
        uint8_t frag_num;                               // number of this fragment
        uint32_t seq_num;                               // sequence number
        const u_char *packet_body;                      // pointer to the first element of the packet payload
    };

    struct dot11VHTMIMOCtrl_frame {
        uint8_t nc;                                     // number of columns
        uint8_t nr;                                     // number of rows
        uint8_t ng;                                     // number of subcarrier groups
        uint16_t channel_width;                         // channel bandwidth
        uint8_t ns;                                     // number of spatial streams (= number of columns)
        uint8_t fb_type;                                // feedback type: SU or MU
        uint8_t codebook;                               /* used codebook of this NDP packet
                                                         * SU-MIMO:
                                                         *  - set to 0 for 2 bits for psi, 4 bits to phi
                                                         *  - set to 1 for 4 bits for psi, 6 bits for phi
                                                         * MU-MIMO:
                                                         *  - set to 0 for 5 bits for psi, 7 bits for phi
                                                         *  - set to 1 for 7 bits for psi, 9 bits for phi
                                                         */
        uint16_t na;                                    // number of angles in the Givens Rotation matrix
        uint16_t nscarrier;                             // number fo subcarriers in the channel
        const u_char *packet_body;                      // pointer to the first element of VHT BF Report Frame
    };

    typedef uint64_t *dot11VHTMIMObf_matrix;

    // Spec 802.11-2013 Sec 8.4.1.48
    struct dot11VHTBFReport_frame {
        float *ASNR;                                    /* average signal to noise ratio (8 bits for each spatial stream)
                                                         * -128 : <= -10dB
                                                         * -127 : -9.75dB
                                                         * -126 : -9.5dB
                                                         * ....   ....
                                                         * +126 : 53.5dB
                                                         * +127 : >= 53.75dB
                                                         */
        dot11VHTMIMObf_matrix *V_array;                 // array of compressed beamforming feedback matrix
        uint8_t na;                                     // number of angles
        uint8_t nscarrier;                              // number of subcarriers in the measuring channel
        uint8_t b_phi;                                  // size of quantized angle phi (in bits)
        uint8_t b_psi;                                  // size of quantized angle psi (in bits)
    };

    struct dot11Header {
        radiotap_header radiotapHeader;                 // radiotap header which is assumed to be appended by driver in mon
        dot11MAC_header dot11MacHeader;                 // 802.11 MAC header
    };

    struct NDPFrame {
        dot11Header *header;
        dot11VHTMIMOCtrl_frame dot11vhtmimoCtrlFrame;   // 802.11 VHT MIMO Control Frame
        dot11VHTBFReport_frame dot11vhtbfReportFrame;   // 802.11 VHT Beamformimng Report Frame
    };

    /**
     * get_dot11_radiotap_header(const u_char*, struct radiotap_header*&)
     * @desc: This function set passed radiotap_header pointer to the location of the transmitted payload,
     *        such that content of radiotap header can be obtained by subsequent process.
     * @param payload : (u_char) pointer to the payload of transmitted radiotap packet
     * @param rtaphdr : (struct radiotap_header*&) auxiliary radiotap header pointer
     * @return uint32 MT_RET_CODE
     */
    MTAPI get_dot11_radiotap_header(const u_char *payload, struct radiotap_header *&rtaphdr);

    /**
     * get_dot11_packet_type(const u_char*)
     * @desc: This function returns the type of transmitted radiotap packet (+1).
     *        According to 802.11-2013 release, this field contains 2 bits (0x3)
     * @param payload : (u_char) pointer to the *mac* payload of transmitted radiotap packet
     * @return uint32 dot11_PACKET_TYPE (0x0: MGMT; 0x1: CTRL; 0x2: DATA)
     */
    MTAPI get_dot11_mac_packet_type(const u_char *payload);

    /**
     * get_dot11_packet_subtype(const u_char*)
     * @desc: This function returns the subtype of transmitted radiotap packet (+0).
     *        According to 802.11-2013 release, this field contains 4 bits (0xf)
     * @param payload : (u_char) pointer to the *mac* payload of transmitted radiotap packet
     * @return uint32 dot11_PACKET_SUBTYPE (0x0-0xe)
     */
    MTAPI get_dot11_mac_packet_subtype(const u_char *payload);

    /**
     * get_dot11_mac_duration(const uchar*)
     * @desc: This function gets the duration field (+2) from transmitted radiotap packet
     *        According to 802.11-2013 release, this field contains 16 bits (0xffff)
     * @param payload : (u_char) pointer to the *mac* payload of transmitted radiotap packet
     * @return uint32
     */
    MTAPI get_dot11_mac_duration(const u_char *payload);

    /**
     * get_dot11_mac_ra(const uchar*)
     * @desc: This function gets the RA field (+4) from transmitted radiotap packet
     *        According to 802.11-2013 release, this field contains 48 bits (0xffffffffffff)
     * @param payload : (u_char) pointer to the *mac* payload of transmitted radiotap packet
     * @return uint64
     */
    MTAPI get_dot11_mac_ra(const u_char *payload);

    /**
     * get_dot11_mac_dst(const uchar*)
     * @desc: This function gets the DST field from transmitted radiotap packet
     *        According to 802.11-2013 release, this field contains 48 bits (0xffffffffffff)
     * @param payload : (u_char) pointer to the *mac* payload of transmitted radiotap packet
     * @return uint64
     */
    MTAPI get_dot11_mac_dst(const u_char *payload);

    /**
     * get_dot11_mac_ta(const uchar*)
     * @desc: This function gets the TA field (+10) from transmitted radiotap packet
     *        According to 802.11-2013 release, this field contains 48 bits (0xffffffffffff)
     * @param payload : (u_char) pointer to the *mac* payload of transmitted radiotap packet
     * @return uint64
     */
    MTAPI get_dot11_mac_ta(const u_char *payload);

    /**
     * get_dot11_mac_bss_id(const uchar*)
     * @desc: This function gets the bss id (+16) from transmitted radiotap packet
     *        According to 802.11-2013 release, this field contains 48 bits (0xffffffffffff)
     * @param payload : (u_char) pointer to the *mac* payload of transmitted radiotap packet
     * @return uint64
     */
    MTAPI get_dot11_mac_bss_id(const u_char *payload);

    /**
     * get_dot11_mac_fragment_num(const uchar*)
     * @desc: This function gets the fragment number (+23) from transmitted radiotap packet
     *        According to 802.11-2013 release, this field contains 4 bits (0xf)
     * @param payload : (u_char) pointer to the *mac* payload of transmitted radiotap packet
     * @return uint8_t
     */
    MTAPI get_dot11_mac_fragment_num(const u_char *payload);

    /**
     * get_dot11_mac_seq_num(const uchar*)
     * @desc: This function gets the sequence number (+22) from transmitted radiotap packet
     *        According to 802.11-2013 release, this field contains 12 bits (0xfff)
     * @param payload : (u_char) pointer to the *mac* payload of transmitted radiotap packet
     * @return uint16_t
     */
    MTAPI get_dot11_mac_seq_num(const u_char *payload);

    /**
     * parse_dot11_mac_header(const u_char*, dot11MAC_header&)
     * @desc: This function reads the content of transmitted radiotap packet and stores in passed
     *        dot11MAC_Header reference.
     * @param payload : (u_char) pointer to the *mac* payload of transmitted radiotap packet
     * @param header : (dot11MAC_header&) reference to the storing mac header data structure
     * @return MT_RET_CODE
     */
    MTAPI parse_dot11_mac_header(const u_char *payload, dot11MAC_header &header);

    /**
     * debug_print_dot11_packet_header_info(dot11Header&)
     * @desc: This function prints all the content of *parsed* variables in radiotap packet to spdlog
     * @param header: reference to the parsed header
     * @return MT_RET_CODE
     */
    MTAPI debug_print_dot11_packet_header_info(dot11Header &header);

    /**
     * debug_print_dot11_ndp_vht_info(NDPFrame&)
     * @desc: This function prints all the content of *parse* variables in MAC MGMT+ACTION_NO_FEEDBACK payload
     * @param frame: reference to the parsed NDP frame
     * @return MT_RET_CODE
     */
    MTAPI debug_print_dot11_ndp_vht_info(NDPFrame &frame);

    MTAPI parse_dot11_packet_header(dot11Header &header, const u_char *payload);

    MTAPI parse_dot11_ndp_vht_control(const u_char *payload, dot11VHTMIMOCtrl_frame &frame);

    MTAPI parse_dot11_ndp_report(dot11Header &header, NDPFrame &frame);

    MTAPI free_dot11_ndp_report(NDPFrame &frame);

    /**
     * decode_dot11_compressed_bf(dot11VHTMIMObf_matrix&)
     * @desc: This function decodes the *parsed* compressed beamforming report matrix V into an N_c by N_c matrix
     * @param comp_mat: compressed beamforming matrix V
     * @param v_arr: reference to the array that stores decoded V matrix (arma::cx_mat) for each subcarrier
     * @return MT_RET_CODE
     */
    MTAPI decode_dot11_compressed_bf(dot11VHTMIMObf_matrix *comp_mats, std::vector<arma::cx_mat> &v_arr,
                                     uint16_t nsubcarrier, uint8_t nc, uint8_t nr, uint8_t b_phi, uint8_t b_psi);

}

#endif //MIMO_TOOLS_DOT11VHTNDP_UTILS_HH
