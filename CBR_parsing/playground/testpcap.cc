//
// Created by Vito Wu on 3/11/2021.
//

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <pcap.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <signal.h>
#include <ctime>

#include "dot11vhtndp_utils.hh"

pcap_t *descr = nullptr;

void printEthernetIIinfo(const struct pcap_pkthdr *header);

void pcapHandler(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);

void exit_handler(int sig) {
    pcap_breakloop(descr);
}

int main() {
    signal(SIGTERM, exit_handler);
    signal(SIGINT, exit_handler);
    pcap_if_t *iface, *devs;
    int i, j;
    char errbuf[PCAP_ERRBUF_SIZE + 1];

    /* Example code to find all available devices using pcap_findalldevs */
    if (pcap_findalldevs(&devs, errbuf) == -1 || !devs) {
        PRINTF_ERR_STAMP("No network devices are currently connected!\n");
        return 1;
    }

    /* Example code to print the information of all network devices */
    PRINTF_STAMP("Enabled Network Devices:\n");
    for (i = 1, iface = devs; iface; iface = iface->next) {
        PRINTF_STAMP("%d - %s\n", i++, iface->name);
    }

    /* Example code to select interface 'en0' and capture live packets */
    pcap_if_t *selected_iface = nullptr;

    char select_iface_name[] = "wlp3s0";
    for (iface = devs; iface; iface = iface->next) {
        if (strcmp(iface->name, select_iface_name) == 0) {
            selected_iface = iface;
            PRINTF_STAMP("Found device: %s\n", select_iface_name);
            break;
        }
    }

    // open device for capture
    /* pcap_t *pcap_open_live(char *device,int snaplen, int prmisc,int to_ms,
       char *ebuf)

       snaplen - maximum size of packets to capture in bytes
       promisc - set card in promiscuous mode?
       to_ms   - time to wait for packets in miliseconds before read
       times out
       errbuf  - if something happens, place error string here
    */
    descr = pcap_open_live(select_iface_name, BUFSIZ, 0, 0, errbuf);
    if (descr == nullptr) {
        PRINTF_STAMP("pcap_open_live(): %s\n", errbuf);
        exit(1);
    }
    // grab a packet from *descr*
    const u_char *packet;

    /*  struct pcap_pkthdr {
        struct timeval ts;   time stamp
        bpf_u_int32 caplen;  length of portion present
        bpf_u_int32;         length this packet (off wire)
        }
     */
    struct pcap_pkthdr hdr{};

    /* u_char *pcap_next(pcap_t *p,struct pcap_pkthdr *h) */
    packet = pcap_next(descr, &hdr);
    if (packet == nullptr) {
        PRINTF_STAMP("Didn't grab packet\n");
        exit(1);
    }
    // printEthernetIIinfo(hdr);

    // create a berkeley packet filter for the handler
    char filter[] = "type mgt";
    bpf_u_int32 netpf = 0;
    struct bpf_program pfp{};
    if (pcap_compile(descr, &pfp, filter, 0, netpf) == -1) {
        PRINTF_ERR_STAMP("failed to compile Libpcap filter, %s\n", filter);
        exit(1);
    }
    if (pcap_setfilter(descr, &pfp) == -1) {
        PRINTF_ERR_STAMP("failed to set Libpcap filter, %s\n", filter);
        exit(1);
    }

    // process each packet in pcapHandler() function
    pcap_loop(descr, 0, pcapHandler, nullptr);
    pcap_freecode(&pfp);
    pcap_close(descr);
    return 0;
}

void pcapHandler(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
    //printEthernetIIinfo(header);

    dot11::dot11Header mac_header{};
    dot11::parse_dot11_packet_header(mac_header, packet);
    if (mac_header.dot11MacHeader.type == dot11_PACKET_TYPE_MGMT &&
        mac_header.dot11MacHeader.subtype == dot11_SUBTYPE_ACTION_NO_ACK &&
        mac_header.dot11MacHeader.RA == 0xe84e069528cdUL) {
//        dot11::debug_print_dot11_packet_header_info(mac_header);
        dot11::NDPFrame frame{};
        dot11::parse_dot11_ndp_report(mac_header, frame);
//        dot11::debug_print_dot11_ndp_vht_info(frame);

        std::vector<arma::cx_mat> v_arr;
        dot11::decode_dot11_compressed_bf(frame.dot11vhtbfReportFrame.V_array, v_arr,
                                          frame.dot11vhtmimoCtrlFrame.nscarrier,
                                          frame.dot11vhtmimoCtrlFrame.nc,
                                          frame.dot11vhtmimoCtrlFrame.nr,
                                          frame.dot11vhtbfReportFrame.b_phi,
                                          frame.dot11vhtbfReportFrame.b_psi);
        v_arr[0].raw_print();
        printf("\n");
        dot11::free_dot11_ndp_report(frame);
    }
}

void printEthernetIIinfo(const struct pcap_pkthdr *header) {
    PRINTF_STAMP("Grabbed packet of length %d\n", header->len);
    PRINTF_STAMP("Received at ..... %s", ctime((const time_t *) &header->ts.tv_sec));
    PRINTF_STAMP("Ethernet address length is %d\n\n", ETHER_HDR_LEN);
}
