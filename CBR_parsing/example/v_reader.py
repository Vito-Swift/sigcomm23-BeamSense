import sys
import socket
import re
from ndpreport_pb2 import NDPReport
from pyargus.antennaArrayPattern import array_rad_pattern, plot_pattern
import matplotlib.pyplot as plt
import numpy as np


def mac_literal_to_string(macint):
    if type(macint) != int:
        raise ValueError('invalid integer')
    return ':'.join(['{}{}'.format(a, b) for a, b in zip(*[iter('{:012x}'.format(macint))] * 2)])


def process_ndp_data(ndp_report):
    print("---- Fetch 1 ndp packet")
    print("---- RA: {0}".format(mac_literal_to_string(ndp_report.mac.RA)))
    print("---- TA: {0}".format(mac_literal_to_string(ndp_report.mac.TA)))
    print("---- Average SNR: ")
    for i in range(len(ndp_report.ASNR)):
        print("\t\tSpatial stream # {0} : {1} dBm".format(i, ndp_report.ASNR[i]))
    print("---- Compressed Beamforming Report")
    for i in range(3):
        print("\t\tSubcarrier # {0}: (".format(i))
        for j in range(ndp_report.steering_matrices[i].row_num):
            for k in range(ndp_report.steering_matrices[i].col_num):
                entry = ndp_report.steering_matrices[i].content[j * ndp_report.steering_matrices[i].row_num + k]
                print("(real: {}   imag: {}) ".format(entry.real, entry.imag))
        print(")")
    print("\t\t..........")


def main():
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.bind(("0.0.0.0", 8000))
        ndpreport = NDPReport()
        while True:
            data, address = s.recvfrom(64 * 1024)
            print("Receive from address: ", address)
            print("Receive %d amount of bytes" % len(data))
            ndpreport.ParseFromString(data)
            process_ndp_data(ndpreport)
            print("")


if __name__ == '__main__':
    N = 4
    d = 0.5
    theta = 73
    w = np.ones(N, dtype=complex)
    for i in np.arange(0, N, 1):
        w[i] = np.exp(i * 1j * 2 * np.pi * d * np.cos(np.deg2rad(theta)))
    x_coords = np.arange(N) * d - (N - 1) * d / 2
    y_coords = np.zeros(N)
    array_alignment = np.array((x_coords, y_coords))

    incident_angles = np.arange(0, 360.1, 0.1)

    surv_ant_pattern = None
    AF_log = array_rad_pattern(array_alignment, incident_angles, w=w, sing_elem_patterns=surv_ant_pattern)
    plt = plot_pattern(incident_angles, AF_log)
    plt.show()

    # main()
