//
// Created by Vito Wu on 13/11/2021.
//

#include "pbuf.hh"
#include "mttool_impl.hh"

using namespace MTOOL_Utils;

namespace MTOOL {

    static pcap_t *descr;

    MTAPI mtool_prep(Environment *env) {

    }

    void mtool_pcap_exit_handler(int sig) {
        pcap_breakloop(descr);
    }

    void mtool_pcap_handler(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
        auto env = (Environment *) args;
        dot11::dot11Header mac_header{};
        dot11::parse_dot11_packet_header(mac_header, packet);

        if (check_filter_rules_aph(&env->rules, &mac_header.dot11MacHeader)
            && mac_header.dot11MacHeader.subtype == dot11_SUBTYPE_ACTION_NO_ACK) {
            PRINTF_STAMP("receive valid ndp report packet\n");

            dot11::NDPFrame frame{};
            dot11::parse_dot11_ndp_report(mac_header, frame);
            std::vector<arma::cx_mat> v_arr;
            dot11::decode_dot11_compressed_bf(frame.dot11vhtbfReportFrame.V_array,
                                              v_arr,
                                              frame.dot11vhtmimoCtrlFrame.nscarrier,
                                              frame.dot11vhtmimoCtrlFrame.nc,
                                              frame.dot11vhtmimoCtrlFrame.nr,
                                              frame.dot11vhtbfReportFrame.b_phi,
                                              frame.dot11vhtbfReportFrame.b_psi);

            if (env->options.verbosity == debug_level::debug) {
                for (int i = 0; i < frame.dot11vhtmimoCtrlFrame.nscarrier; i++) {
                    PRINTF_STAMP("\tSubcarrier %d: \n", i);
                    for (int j = 0; j < v_arr[i].n_rows; j++) {
                        PRINTF_STAMP("\t\t");
                        for (int k = 0; k < v_arr[i].n_cols; k++)
                            printf("%lf\t", v_arr[i](j, k));
                        printf("\n");
                    }
                }
                printf("\n");
            }

            if (env->options.o_hostname != nullptr) {
                pbuf_interface::write_ndp_report_to_udp(frame, v_arr, env->options.o_hostname, env->options.o_port);
            } else {
                pbuf_interface::write_ndp_report_to_udp(frame, v_arr, "localhost", env->options.o_port);
            }
            dot11::free_dot11_ndp_report(frame);
        }
    }

    MTAPI mtool_pcap_entry(Environment *env) {
        pcap_if_t *iface, *devs;
        int i, j;
        char errbuf[PCAP_ERRBUF_SIZE + 1];

        if (pcap_findalldevs(&devs, errbuf) == -1 || !devs) {
            PRINTF_ERR_STAMP("No network devices are currently connected!\n");
            return MT_RET_FAIL;
        }

        // pre-check available network interfaces
        if (env->options.verbosity == debug_level::debug) {
            PRINTF_STAMP("Enabled Network Devices:\n");
            for (i = 1, iface = devs; iface; iface = iface->next) {
                PRINTF_STAMP("%d - %s\n", i++, iface->name);
            }
        }

        // select from available network ifaces
        bool iface_found = false;
        for (iface = devs; iface; iface = iface->next) {
            if (strcmp(iface->name, env->options.iface) == 0) {
                env->iface = iface;
                iface_found = true;
                if (env->options.verbosity == debug_level::debug)
                    PRINTF_STAMP("Found device: %s\n", env->options.iface);
                break;
            }
        }
        if (!iface_found) {
            PRINTF_ERR_STAMP("Cannot found device: %s\n", env->options.iface);
            return MT_RET_FAIL;
        }

        // open device for capture
        env->descr = pcap_open_live(env->options.iface, BUFSIZ, 0, 0, errbuf);
        if (env->descr == nullptr) {
            PRINTF_ERR_STAMP("pcap_open_live(): %s\n", errbuf);
            return MT_RET_FAIL;
        }

        // set exit handler
        descr = env->descr;
        signal(SIGTERM, mtool_pcap_exit_handler);
        signal(SIGINT, mtool_pcap_exit_handler);
        signal(SIGKILL, mtool_pcap_exit_handler);

        // create a bpf for the handler
        char filter[] = "type mgt";
        bpf_u_int32 netpf = 0;
        struct bpf_program pfp{};
        if (pcap_compile(env->descr, &pfp, filter, 0, netpf) == -1) {
            PRINTF_ERR_STAMP("failed to compile Libpcap filter, %s\n", filter);
            return MT_RET_FAIL;
        }
        if (pcap_setfilter(env->descr, &pfp) == -1) {
            PRINTF_ERR_STAMP("failed to set libpcap filter, %s\n", filter);
            return MT_RET_FAIL;
        }

        pcap_loop(descr, 0, mtool_pcap_handler, (u_char *) (env));
        pcap_freecode(&pfp);
        pcap_close(descr);

        return MT_RET_SUCCESS;
    }

    MTAPI mtool_8814au_entry(Environment *env) {

    }

}
