//
// Created by Vito Wu on 4/11/2021.
//

#include "dot11vhtndp_utils.hh"

namespace dot11 {

    MTAPI get_dot11_radiotap_header(const u_char *payload, struct radiotap_header *&rtaphdr) {
        rtaphdr = (struct radiotap_header *) payload;
        return MT_RET_SUCCESS;
    }

    MTAPI get_radiotap_header_len(const u_char *payload) {
        auto rtaphdr = (struct radiotap_header *) payload;
        return (rtaphdr->it_len);
    }

    MTAPI parse_radiotap_header(const u_char *payload, struct radiotap_header &radiotapHeader) {
        struct radiotap_header *rtaphdr;
        get_dot11_radiotap_header(payload, rtaphdr);
        radiotapHeader.it_len = rtaphdr->it_len;
        radiotapHeader.it_pad = rtaphdr->it_pad;
        radiotapHeader.it_rev = rtaphdr->it_rev;
        return MT_RET_SUCCESS;
    }

    //  Type/Subtype layout: || Subtype (4 bits) | Type (2 bits) | Version (2 bits) ||
    MTAPI get_dot11_mac_packet_type(const u_char *payload) {
        const u_char *pptype = payload;
        uint8_t packet_type = ((*pptype) >> 2) & 0x3;
        return packet_type;
    }

    //  Type/Subtype layout: || Subtype (4 bits) | Type (2 bits) | Version (2 bits) ||
    MTAPI get_dot11_mac_packet_subtype(const u_char *payload) {
        const u_char *pptype = payload;
        uint8_t sub_packet_type = ((*pptype) >> 4);
        return sub_packet_type;
    }

    MTAPI get_dot11_mac_duration(const u_char *payload) {
        // TODO: because we are not using duration information, so the function implementation is dummy
        return 0;
    }

    MTAPI get_dot11_mac_ra(const u_char *payload) {
        const u_char *pra = payload + 4;
        return (((uint64_t) *pra << 40) |
                ((uint64_t) *(pra + 1) << 32) |
                ((uint64_t) *(pra + 2) << 24) |
                ((uint64_t) *(pra + 3) << 16) |
                ((uint64_t) *(pra + 4) << 8) |
                ((uint64_t) *(pra + 5)));
    }

    MTAPI get_dot11_mac_dst(const u_char *payload) {
        return get_dot11_mac_ra(payload);
    }

    MTAPI get_dot11_mac_ta(const u_char *payload) {
        const u_char *pta = payload + 10;
        return (((uint64_t) *pta << 40) |
                ((uint64_t) *(pta + 1) << 32) |
                ((uint64_t) *(pta + 2) << 24) |
                ((uint64_t) *(pta + 3) << 16) |
                ((uint64_t) *(pta + 4) << 8) |
                ((uint64_t) *(pta + 5)));
    }

    MTAPI get_dot11_mac_bss_id(const u_char *payload) {
        const u_char *pbi = payload + 16;
        return (((uint64_t) *pbi << 40) |
                ((uint64_t) *(pbi + 1) << 32) |
                ((uint64_t) *(pbi + 2) << 24) |
                ((uint64_t) *(pbi + 3) << 16) |
                ((uint64_t) *(pbi + 4) << 8) |
                ((uint64_t) *(pbi + 5)));
    }

    //  Frag/seq layout: || Subtype (12 bits) | Version (4 bits) ||
    MTAPI get_dot11_mac_fragment_num(const u_char *payload) {
        const u_char *pfn = payload + 23;
        return (uint8_t) (*pfn & 0xf);
    }

    //  Frag/seq layout: || Subtype (12 bits) | Version (4 bits) ||
    MTAPI get_dot11_mac_seq_num(const u_char *payload) {
        const u_char *psn = payload + 22;
        return (uint16_t) ((uint8_t) (*psn) << 4) | ((uint8_t) *(psn + 1) >> 4);
    }

    const u_char *get_dot11_mac_payload_body(const u_char *payload) {
        return payload + 24;
    }

    MTAPI parse_dot11_mac_header(const u_char *payload, dot11MAC_header &header) {
        if (payload == nullptr) {
            PRINTF_STAMP("802.11 MAC payload is unreferenced, parse failed!\n");
            return MT_RET_FAIL;
        }

        header.type = get_dot11_mac_packet_type(payload);
        header.subtype = get_dot11_mac_packet_subtype(payload);
        header.RA = get_dot11_mac_ra(payload);
        header.TA = get_dot11_mac_ta(payload);
        header.DST = get_dot11_mac_dst(payload);
        header.BSS = get_dot11_mac_bss_id(payload);
        header.frag_num = get_dot11_mac_fragment_num(payload);
        header.seq_num = get_dot11_mac_seq_num(payload);
        header.packet_body = get_dot11_mac_payload_body(payload);

        // PRINTF_STAMP("802.11 MAC payload is parsed.\n");
        return MT_RET_SUCCESS;
    }

    std::string debug_convert_mac64_to_str(const uint64_t mac) {
        std::stringstream s;
        s << std::hex << mac;
        std::stringstream rets;
        rets << s.str()[0] << s.str()[1];
        for (int i = 2; i < s.str().size() - 1; i += 2)
            rets << '-' << s.str()[i] << s.str()[i + 1];
        return rets.str();
    }

    std::string debug_convert_type_to_literal(const uint64_t packet_type) {
        if (packet_type == dot11_PACKET_TYPE_MGMT)
            return std::string("MGMT");

        else if (packet_type == dot11_PACKET_TYPE_CTRL)
            return std::string("CTRL");

        else if (packet_type == dot11_PACKET_TYPE_DATA)
            return std::string("DATA");

        return std::string("UNKNOWN");
    }

    std::string debug_convert_subtype_to_literal(const uint64_t packet_type, const uint64_t subpacket_type) {
        if (packet_type == dot11_PACKET_TYPE_MGMT) {

            if (subpacket_type == dot11_SUBTYPE_ASSO_REQ)
                return std::string("ASSOCIATION REQUEST");

            else if (subpacket_type == dot11_SUBTYPE_ASSO_RES)
                return std::string("ASSOCIATION RESPONSE");

            else if (subpacket_type == dot11_SUBTYPE_REASSO_REQ)
                return std::string("RE-ASSOCIATION REQUEST");

            else if (subpacket_type == dot11_SUBTYPE_REASSO_RES)
                return std::string("RE-ASSOCIATION RESPONSE");

            else if (subpacket_type == dot11_SUBTYPE_PROBE_REQ)
                return std::string("PROBE REQUEST");

            else if (subpacket_type == dot11_SUBTYPE_PROBE_RES)
                return std::string("PROBE RESPONSE");

            else if (subpacket_type == dot11_SUBTYPE_BEACON)
                return std::string("BEACON");

            else if (subpacket_type == dot11_SUBTYPE_ATIM)
                return std::string("ATIM");

            else if (subpacket_type == dot11_SUBTYPE_DISASSO)
                return std::string("DISASSOCIATION");

            else if (subpacket_type == dot11_SUBTYPE_AUTH)
                return std::string("AUTHENTICATION");

            else if (subpacket_type == dot11_SUBTYPE_DEAUTH)
                return std::string("DEAUTHENTICATION");

            else if (subpacket_type == dot11_SUBTYPE_ACTION_NO_ACK)
                return std::string("ACTION NO ACK");
        }
        return std::string("UNKNOWN");
    }

    uint64_t debug_convert_bandwidth_to_literal(const uint64_t bandwidth) {
        if (bandwidth == 0x0)
            return 20;
        else if (bandwidth == 0x1)
            return 40;
        else if (bandwidth == 0x2)
            return 80;
        else if (bandwidth == 0x3)
            return 160;
        return 0;
    }

    std::string debug_convert_fb_to_literal(const uint64_t feedback) {
        if (feedback == 0x0)
            return std::string("SU");
        else if (feedback == 0x1)
            return std::string("MU");
        return std::string("UNKNOWN");
    }

    MTAPI debug_print_dot11_packet_header_info(dot11Header &header) {
        PRINTF_STAMP("Print radiotap header info:\n"
                     "\t\t\t\tradiotap version: %d\n"
                     "\t\t\t\tpadding: %d\n"
                     "\t\t\t\theader length: %d\n",
                     header.radiotapHeader.it_rev, header.radiotapHeader.it_pad,
                     header.radiotapHeader.it_len);

        PRINTF_STAMP("Print mac header info:\n"
                     "\t\t\t\ttype: %d (%s)\n"
                     "\t\t\t\tsubtype: %d (%s)\n"
                     "\t\t\t\treceiver address: %s\n"
                     "\t\t\t\ttransmitter address: %s\n"
                     "\t\t\t\tbss id: %s\n"
                     "\t\t\t\tfragment num: %d\n"
                     "\t\t\t\tsequence num: %d\n",
                     header.dot11MacHeader.type,
                     debug_convert_type_to_literal(header.dot11MacHeader.type).c_str(),
                     header.dot11MacHeader.subtype,
                     debug_convert_subtype_to_literal(header.dot11MacHeader.type,
                                                      header.dot11MacHeader.subtype).c_str(),
                     debug_convert_mac64_to_str(header.dot11MacHeader.RA).c_str(),
                     debug_convert_mac64_to_str(header.dot11MacHeader.TA).c_str(),
                     debug_convert_mac64_to_str(header.dot11MacHeader.BSS).c_str(),
                     header.dot11MacHeader.frag_num, header.dot11MacHeader.seq_num);

        return MT_RET_SUCCESS;
    }

    MTAPI debug_print_dot11_ndp_vht_info(NDPFrame &frame) {
        PRINTF_STAMP("Print VHT Control Field:\n"
                     "\t\t\t\t# columns: %d\n"
                     "\t\t\t\t# rows: %d\n"
                     "\t\t\t\t# subcarrier groups: %d\n"
                     "\t\t\t\t# subcarriers in compressed beamforming report: %d\n"
                     "\t\t\t\tchannel width: %d (%luMhz)\n"
                     "\t\t\t\t# spatial streams: %d\n"
                     "\t\t\t\tfeedback type: %d (%s)\n"
                     "\t\t\t\tcodebook: %d\n"
                     "\t\t\t\t# angles: %d\n",
                     frame.dot11vhtmimoCtrlFrame.nc,
                     frame.dot11vhtmimoCtrlFrame.nr,
                     frame.dot11vhtmimoCtrlFrame.ng,
                     frame.dot11vhtmimoCtrlFrame.nscarrier,
                     frame.dot11vhtmimoCtrlFrame.channel_width,
                     debug_convert_bandwidth_to_literal(frame.dot11vhtmimoCtrlFrame.channel_width),
                     frame.dot11vhtmimoCtrlFrame.ns,
                     frame.dot11vhtmimoCtrlFrame.fb_type,
                     debug_convert_fb_to_literal(frame.dot11vhtmimoCtrlFrame.fb_type).c_str(),
                     frame.dot11vhtmimoCtrlFrame.codebook,
                     frame.dot11vhtmimoCtrlFrame.na);

        std::stringstream asnr_str;
        asnr_str.precision(4);
        asnr_str << "Print ASNR: \n";
        for (int i = 0; i < frame.dot11vhtmimoCtrlFrame.ns; i++) {
            asnr_str << "\t\t\t\tSpatial Stream #" << i
                     << " : " << frame.dot11vhtbfReportFrame.ASNR[i] << "dB\n";
        }
        PRINTF_STAMP("%s", asnr_str.str().c_str());

        return MT_RET_SUCCESS;
    }


    MTAPI get_dot11_ndp_vht_nc(const u_char *payload, dot11VHTMIMOCtrl_frame &frame) {
        const u_char *pnc = payload + 2;
        return (((*pnc) & 0x7)) + 1;         // in vht control field, literal nc is minus by 1
    }

    MTAPI get_dot11_ndp_vht_nr(const u_char *payload, dot11VHTMIMOCtrl_frame &frame) {
        const u_char *pnc = payload + 2;
        return (((*pnc) >> 3) & 0x7) + 1;   // in vht control field, literal nr is minus by 1
    }

    MTAPI get_dot11_ndp_vht_CW(const u_char *payload, dot11VHTMIMOCtrl_frame &frame) {
        const u_char *pcw = payload + 2;
        return ((*pcw) >> 6) & 0x3;
    }

    MTAPI get_dot11_ndp_vht_fb_type(const u_char *payload, dot11VHTMIMOCtrl_frame &frame) {
        const u_char *pfb = payload + 3;
        return ((*pfb) >> 3) & 0x1;
    }

    MTAPI get_dot11_ndp_vht_cb(const u_char *payload, dot11VHTMIMOCtrl_frame &frame) {
        const u_char *pcb = payload + 3;
        return ((*pcb) >> 2) & 0x1;
    }

    MTAPI get_dot11_ndp_vht_ng(const u_char *payload, dot11VHTMIMOCtrl_frame &frame) {
        const u_char *png = payload + 3;
        return (*png) & 0x3;
    }

    MTAPI get_dot11_ndp_vht_na(dot11VHTMIMOCtrl_frame &frame) {
        const uint8_t nc = frame.nc;
        const uint8_t nr = frame.nr;

        if (!(((nc >= 1) && (nc <= 4)) || ((nr >= 1) && (nr <= 4)))) { // filter out case when > 4x4 mimo
            PRINTF_ERR_STAMP("[!] cannot recognize case when nc || nr >= 4 or == 0\n");
            return 0x0;
        }

        if (nc > nr) {
            PRINTF_ERR_STAMP("[!] cannot recognize case when nc > nr\n");
            return 0x0;
        }

        if (nr == 0x2)      // cover both 2x1 and 2x2
            return 0x2;
        else if (nr == 0x3) {
            if (nc == 0x1)
                return 0x4;
            else if (nc == 0x2 || nc == 0x3)
                return 0x6;
        } else if (nr == 0x4) {
            if (nc == 0x1)
                return 0x6;
            else if (nc == 0x2)
                return 0xa; // 0d10
            else if (nc == 0x3 || nc == 0x4)
                return 0xc; // 0d12
        }

        PRINTF_ERR_STAMP("[!] unrecognized nc: %d \t nr: %d\n", nc, nr);
        return 0x0;
    }

    // defined in 802.11-2013 Table 8-53g (p.59)
    MTAPI get_dot11_ndp_vht_nsubcarrier(dot11VHTMIMOCtrl_frame &frame) {
        const uint8_t ng = frame.ng;
        const uint16_t channel_width = frame.channel_width;

        switch (channel_width) {

            case 0x00: { // channel width = 20MHz
                if (ng == 0)
                    return 52;
                else if (ng == 1)
                    return 30;
                else if (ng == 2)
                    return 16;
                else goto UNREC_NG;
            }

            case 0x01: { // channel width = 40MHz
                if (ng == 0)
                    return 108;
                else if (ng == 1)
                    return 58;
                else if (ng == 2)
                    return 30;
                else goto UNREC_NG;
            }

            case 0x02: { // channel width = 80MHz
                if (ng == 0)
                    return 234;
                else if (ng == 1)
                    return 122;
                else if (ng == 2)
                    return 62;
                else goto UNREC_NG;
            }

            case 0x03: { // channel width = 160MHz or 80+80 MHz
                if (ng == 0)
                    return 468;
                else if (ng == 1)
                    return 244;
                else if (ng == 2)
                    return 124;
                else goto UNREC_NG;
            }

            default:
                goto UNREC_NG;
        }

        UNREC_NG:
        PRINTF_ERR_STAMP("[!] unrecognized cw: %d \t ng: %d\n ", channel_width, ng);
        return 0x0;
    }

    const u_char *get_dot11_ndp_vht_packet_body(const u_char *payload, dot11VHTMIMOCtrl_frame &frame) {
        return payload + 5;
    }

    MTAPI parse_dot11_packet_header(dot11Header &header, const u_char *payload) {
        parse_radiotap_header(payload, header.radiotapHeader);
        uint32_t offset_r2m = header.radiotapHeader.it_len;
        parse_dot11_mac_header(payload + offset_r2m, header.dot11MacHeader);
        return MT_RET_SUCCESS;
    }

    MTAPI parse_dot11_ndp_vht_control(const u_char *payload, dot11VHTMIMOCtrl_frame &frame) {
        if (payload == nullptr) {
            PRINTF_ERR_STAMP("802.11 VHT NDP report payload is unreferenced, parse failed!\n");
            return MT_RET_FAIL;
        }

        frame.nc = get_dot11_ndp_vht_nc(payload, frame);
        frame.nr = get_dot11_ndp_vht_nr(payload, frame);
        frame.channel_width = get_dot11_ndp_vht_CW(payload, frame);
        frame.fb_type = get_dot11_ndp_vht_fb_type(payload, frame);
        frame.codebook = get_dot11_ndp_vht_cb(payload, frame);
        frame.ng = get_dot11_ndp_vht_ng(payload, frame);
        frame.ns = frame.nc;
        frame.na = get_dot11_ndp_vht_na(frame);
        frame.nscarrier = get_dot11_ndp_vht_nsubcarrier(frame);
        frame.packet_body = get_dot11_ndp_vht_packet_body(payload, frame);

        return MT_RET_SUCCESS;
    }

    MTAPI get_dot11_ndp_vht_bphi(NDPFrame &frame) {
        if (frame.dot11vhtmimoCtrlFrame.fb_type == dot11_VHT_NDP_FEEDBACK_SU) {
            if (frame.dot11vhtmimoCtrlFrame.codebook == 0)
                return 4;
            else if (frame.dot11vhtmimoCtrlFrame.codebook == 1)
                return 6;
        }

        if (frame.dot11vhtmimoCtrlFrame.fb_type == dot11_VHT_NDP_FEEDBACK_MU) {
            if (frame.dot11vhtmimoCtrlFrame.codebook == 0)
                return 7;
            else if (frame.dot11vhtmimoCtrlFrame.codebook == 1)
                return 9;
        }

        PRINTF_ERR_STAMP("[!] Unrecognized feedback type: %d\tcodebook: %d\n",
                         frame.dot11vhtmimoCtrlFrame.fb_type,
                         frame.dot11vhtmimoCtrlFrame.codebook);
        return 0;
    }

    MTAPI get_dot11_ndp_vht_bpsi(NDPFrame &frame) {
        if (frame.dot11vhtmimoCtrlFrame.fb_type == dot11_VHT_NDP_FEEDBACK_SU) {
            if (frame.dot11vhtmimoCtrlFrame.codebook == 0)
                return 2;
            else if (frame.dot11vhtmimoCtrlFrame.codebook == 1)
                return 4;
        }

        if (frame.dot11vhtmimoCtrlFrame.fb_type == dot11_VHT_NDP_FEEDBACK_MU) {
            if (frame.dot11vhtmimoCtrlFrame.codebook == 0)
                return 5;
            else if (frame.dot11vhtmimoCtrlFrame.codebook == 1)
                return 7;
        }

        PRINTF_ERR_STAMP("[!] Unrecognized feedback type: %d\tcodebook: %d\n",
                         frame.dot11vhtmimoCtrlFrame.fb_type,
                         frame.dot11vhtmimoCtrlFrame.codebook);
        return 0;
    }

    // This function calloc sufficient memory space to store the compressed beamforming report
    MTAPI init_dot11_ndp_vht_compressed_bf_report(NDPFrame &frame) {
        // allocate Nc * sizeof float cells for storing ANSR report
        frame.dot11vhtbfReportFrame.ASNR = SMALLOC(float, frame.dot11vhtmimoCtrlFrame.nc);

        // allocate Nsubcarrier * size of (compressed beamforming matrix V)
        frame.dot11vhtbfReportFrame.V_array = SMALLOC(dot11VHTMIMObf_matrix, frame.dot11vhtmimoCtrlFrame.nscarrier);
        frame.dot11vhtbfReportFrame.b_phi = get_dot11_ndp_vht_bphi(frame);
        frame.dot11vhtbfReportFrame.b_psi = get_dot11_ndp_vht_bpsi(frame);
        for (int i = 0; i < frame.dot11vhtmimoCtrlFrame.nscarrier; i++) {
            frame.dot11vhtbfReportFrame.V_array[i] = SMALLOC(uint64_t, frame.dot11vhtmimoCtrlFrame.na);
        }

        return MT_RET_SUCCESS;
    }

    MTAPI free_dot11_ndp_vht_compressed_bf_report(NDPFrame &frame) {
        SFREE(frame.dot11vhtbfReportFrame.ASNR);
        for (int i = 0; i < frame.dot11vhtmimoCtrlFrame.nscarrier; i++)
            SFREE(frame.dot11vhtbfReportFrame.V_array[i]);
        SFREE(frame.dot11vhtbfReportFrame.V_array);
        return MT_RET_SUCCESS;
    }

    // defined in 802.11-2013 table 8-53h (p.64)
    MTAPI parse_dot11_ndp_vht_compressed_bf_report_ASNR(NDPFrame &frame) {
        for (int i = 0; i < frame.dot11vhtmimoCtrlFrame.ns; i++) { // iterate over each spatial streams
            int8_t comp_asnr = *(int8_t *) (frame.dot11vhtmimoCtrlFrame.packet_body + i);
            frame.dot11vhtbfReportFrame.ASNR[i] = -10.0 + (comp_asnr + 128) * 0.25;
        }

        return MT_RET_SUCCESS;
    }

    // defined in 802.11-2013 table 8-53d (p.55)
    MTAPI parse_dot11_ndp_vht_compressed_bf_report_V(NDPFrame &frame) {
        // beamforming feedback matrix is stored after the ASNR fields
        u_char *v_start_point = (u_char *) (frame.dot11vhtmimoCtrlFrame.packet_body + frame.dot11vhtmimoCtrlFrame.ns);
        uint8_t bit_offset = 0;

        // get the entry idx in the angle_representation_table
        const uint8_t entry_idx = ((2 + frame.dot11vhtmimoCtrlFrame.nc) * (frame.dot11vhtmimoCtrlFrame.nc - 2) / 2)
                                  + frame.dot11vhtmimoCtrlFrame.nr - 1;

//        // initialize bitset
//        size_t num_bits_of_V = 0;
//        size_t num_bytes_of_V = num_bits_of_V % 8 ? (num_bits_of_V / 8) + 1 : num_bits_of_V;
//        for (int i = 0; i < frame.dot11vhtmimoCtrlFrame.na; i++) {
//            // iterate all angles in table and sum the size
//            if (angle_representation_table[entry_idx][i] == dot11_VHT_NDP_ANGLE_PSI)
//                num_bits_of_V += frame.dot11vhtbfReportFrame.b_psi;
//            if (angle_representation_table[entry_idx][i] == dot11_VHT_NDP_ANGLE_PHI)
//                num_bits_of_V += frame.dot11vhtbfReportFrame.b_phi;
//        }

        for (int i = 0; i < frame.dot11vhtmimoCtrlFrame.nscarrier; i++) { // iterate over each subcarrier
            for (int j = 0; j < frame.dot11vhtmimoCtrlFrame.na; j++) { // iterate over each angle
                uint16_t val = 0;

                uint8_t angle_size;     // size of current angle (in bits)
                if (angle_representation_table[entry_idx][j] == dot11_VHT_NDP_ANGLE_PSI)
                    angle_size = frame.dot11vhtbfReportFrame.b_psi;
                if (angle_representation_table[entry_idx][j] == dot11_VHT_NDP_ANGLE_PHI)
                    angle_size = frame.dot11vhtbfReportFrame.b_phi;

                bool cross_bytes = bit_offset + angle_size >= 8;
                if (cross_bytes) {  // in different byte entries
                    uint8_t num_bits_curt_byte = 8 - bit_offset;
                    uint8_t num_bits_next_byte = bit_offset + angle_size - 8;
                    uint8_t curt_mask = (~(~0u << num_bits_curt_byte)) << bit_offset;  //  set lowest 'cmask' bits to 1
                    uint8_t next_mask = ~(~0u << num_bits_next_byte);                  //  set highest 'nmask' bits to 1

                    val = (((*v_start_point) & curt_mask) >> bit_offset) |
                          (((*(v_start_point + 1)) & next_mask) << num_bits_next_byte);

                    bit_offset += angle_size;
                    bit_offset -= 8;
                    v_start_point++;    // move to next byte
                } else {   // in the same byte entry
                    uint8_t mask = (~(~0u << angle_size));
                    val = ((*v_start_point) >> bit_offset) & mask;
                    bit_offset += angle_size;
                }

                frame.dot11vhtbfReportFrame.V_array[i][j] = val;
            }
        }

        return MT_RET_SUCCESS;
    }

    MTAPI parse_dot11_ndp_vht_compressed_bf_report(NDPFrame &frame) {
        if (frame.dot11vhtmimoCtrlFrame.packet_body == nullptr) {
            PRINTF_ERR_STAMP("802.11 VHT NDP Compressed Beamforming Report field is unreferenced, parse failed!\n");
            return MT_RET_FAIL;
        }

        init_dot11_ndp_vht_compressed_bf_report(frame);
        parse_dot11_ndp_vht_compressed_bf_report_ASNR(frame);
        parse_dot11_ndp_vht_compressed_bf_report_V(frame);

        return MT_RET_SUCCESS;
    }

    MTAPI parse_dot11_ndp_report(dot11Header &header, NDPFrame &frame) {
        frame.header = &header;
        parse_dot11_ndp_vht_control(header.dot11MacHeader.packet_body, frame.dot11vhtmimoCtrlFrame);
        parse_dot11_ndp_vht_compressed_bf_report(frame);
        return MT_RET_SUCCESS;
    }

    MTAPI free_dot11_ndp_report(NDPFrame &frame) {
        free_dot11_ndp_vht_compressed_bf_report(frame);
        return MT_RET_SUCCESS;
    }

    double decode_compressed_bf_phi(uint64_t entry, uint8_t b_phi) {
        return (((double) entry) / ((double) (1 << (b_phi + 1)))
                + (1 / (double) (1 << (b_phi + 2)))) * M_PI;
    }

    double decode_compressed_bf_psi(uint64_t entry, uint8_t b_psi) {
        return (((double) entry) / ((double) (1 << (b_psi + 1)))
                + (1 / (double) (1 << (b_psi + 2)))) * M_PI;
    }

    // decoding procedure is defined in 802.11-2009 section 20.3.12.2.5 (p.308)
    // The compressed beamforming matrix V = [\prod_{1,...,min(N_c, N_r - 1)} [D_i \cdot \prod{G}]] \cdot I_{NrxNc}
    //
    // Matrix D_i is an Nr \times Nr diagonal matrix, where
    //  D_i = D(1_{i-1}, e^{j * \phi_{i,i}}, ..., e^{j * \phi_{N_r - 1}, i}, 1)
    //      = |  I_{i - 1}  ,    0             ,      ...      ,         ...             ,     0      |
    //        |      0      , e^{j*\phi_{i,i}} ,       0       ,         ...             ,     0      |
    //        |      :      ,    0             ,      `.       ,          0              ,     0      |
    //        |      :      ,    :             ,        `.     ,          :              ,     :      |
    //        |      :      ,    :             ,       0       ,  e^{j*\phi_{N_r-1, i}}  ,     0      |
    //        |      0      ,    0             ,       0       ,          0              ,     1      |
    //
    //
    // Matrix G_li (\psi) is an Nr \times Nr Givens Rotation matrix, where
    //  G_{li} = |  I_{i-1}  ,     0      ,      0      ,      0      ,    0      |
    //           |    0      , cos(\psi)  ,      0      , sine(\psi)  ,    0      |
    //           |    0      ,     0      ,  I_{l-i-1}  ,      0      ,    0      |
    //           |    0      , -sin(\psi) ,      0      ,  cos(\psi)  ,    0      |
    //           |    0      ,     0      ,      0      ,      0      , I_{N_r-l} |
    MTAPI decode_dot11_compressed_bf(dot11VHTMIMObf_matrix *comp_mats, std::vector<arma::cx_mat> &v_arr,
                                     uint16_t nsubcarrier, uint8_t nc, uint8_t nr, uint8_t b_phi, uint8_t b_psi) {
        v_arr.clear();
        v_arr.resize(nsubcarrier);

        for (int s = 0; s < nsubcarrier; s++) {    // iterate over each subcarrier
            arma::cx_mat lhs(nr, nr, arma::fill::eye); // initial lhs result with an identity matrix

            for (int i = 0; i < MIN(nc, nr - 1); i++) {
                int16_t comp_mat_p = 0;
                arma::cx_mat D_i(nr, nr, arma::fill::eye);
                for (int ni = i; ni < nr; ni++) {
                    double phi = decode_compressed_bf_phi(comp_mats[s][comp_mat_p++], b_phi);
                    D_i(ni, ni) = std::complex<double>(cos(phi), sin(phi));
                }

                arma::cx_mat G(nr, nr, arma::fill::eye);  // initial matrix G with Nr x Nr identity
                for (int l = i + 1; l < nr; l++) {
                    double psi = decode_compressed_bf_psi(comp_mats[s][comp_mat_p++], b_psi);
                    arma::cx_mat G_li(nr, nr, arma::fill::eye);
                    G_li(i, i) = cos(psi);
                    G_li(l - i, l - i) = cos(psi);
                    G_li(l - i, i) = -1 * sin(psi);
                    G_li(i, l - i) = sin(psi);
                    G *= G_li;
                }
                lhs *= (D_i * G);
            }
            v_arr[s] = lhs * arma::cx_mat(nr, nc, arma::fill::eye);
        }

        return MT_RET_SUCCESS;
    }

}
