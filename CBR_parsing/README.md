# VHT-Toolkit

Visualization and analyze tools for 802.11ac WiFi

---

## System Requirements

To build this project, one should ensure that following third-party libraries are installed on the system

- spdlog library: [follow this link to install](https://github.com/gabime/spdlog)
- pcap library: [follow this link to install](https://www.tcpdump.org/)
- boost library: [follow this link to install](https://www.boost.org/)
- Armadillo library: [follow this link to install](http://arma.sourceforge.net/)
- Protobuf: [follow this link to install](https://github.com/protocolbuffers/protobuf/blob/master/src/README.md)

---

## Kick Start

Our project provides two ways to capture and analyze 802.11ac NDP/CSI packets:

### Use in eavesdropper mode (with *libpcap*)

First, you need to **enable the monitor mode** of your wireless interface.

On most of debian-like OS, the network manager need to be shut down before allowing a wireless card initial monitor mode
and capture network traffic from the air:

```shell
# Stop network manager
$ sudo systemctl stop NetworkManager.service

# enable monitor mode of wlan0
$ sudo ifconfig wlan0 down
$ sudo iwconfig wlan0 mode monitor
$ sudo ifconfig wlan0 up
$ sudo iwconfig wlan0 channel 36  # to monitor traffic of 5.18GHz
```

Alternatively, you can also use aircrack-ng to start monitor mode:

```shell
# Install aircrack-ng toolkits
$ sudo apt update && sudo apt install -y aircrack-ng

# Check and kill all the processes that are occupying assigned wireless interface
$ sudo airmon-ng check kill

# Start monitor mode
$ sudo airmon-ng start wlan0

# Then you can check the captured packet on channel 36
$ sudo airodump-ng -c 36 wlan0mon
# You can now monitor wlan0mon on wireshark to see the detail of each dumped packet
```

### Build the project

```shell
$ mkdir build && cd build && cmake ..
```

### Launch MT_Tools

After compilation, a binary executable `mt_tool` will be generated under `$build` folder. MT_tools need to read several
command line parameters when starting

```text
--verbose : print extra debug message

--mode [MODE] : 'pcap' or '8814au'

--iface [INTERFACE] : set wireless interface to use, e.g., 'wlan1', 'wlp2s0', etc

--filter [EXP1,EXP2,EXP3,...,EXPN] : set filter on capturing packet, 
                                     e.g. to only capture receiver with specific mac 
                                     address: RA=xx:xx:xx:xx:xx:xx

--host [HOSTNAME] / --port [PORT] : IP address and port to send protobuf UDP packet

--help : show help message
```

For example, to launch eavesdropper mode

```text
$ sudo ./mt_tool --mode pcap --iface wlp3s0 --host 127.0.0.1 --port 8000 --filter RA=e8:4e:06:95:28:cd
```

If there is any valid NDP packet (with receiver address match `e8:4e:06:95:28:cd` ) listened by network
interface `wlp3s0`, the packet will be then parsed as a protobuf packet and send to `127.0.0.1:8000` by UDP:

```text
$ sudo ./mt_tool --mode pcap --iface wlp3s0 --host 127.0.0.1 --port 8000 --filter RA=e8:4e:06:95:28:cd
        0.00 - option mode libpcap
        0.00 - 		option interface: wlp3s0
        0.00 - 		host ip: 127.0.0.1
        0.00 - 		option port: 8000
        0.00 - 		raw filter options: RA=e8:4e:06:95:28:cd
        0.00 - Parse filter rules:
        0.00 - 		retrieved 1 filter expressions
        0.00 - 		- RA=e8:4e:06:95:28:cd	[EQUAL_EXP]
        0.00 - Enter operation mode: pcap eavesdropper
        6.66 - receive valid ndp report packet
        6.67 - Sent one packet: 127.0.0.1 : 8000
       10.74 - receive valid ndp report packet
       10.75 - Sent one packet: 127.0.0.1 : 8000
       14.82 - receive valid ndp report packet
       14.83 - Sent one packet: 127.0.0.1 : 8000
```

In `example/v_reader.py`, we provide an example on how to listen UDP port and receive this protobuf packet.

---

### Some working notes
