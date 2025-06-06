import numpy as np

# Reads the data file
def readFile(filename):

    f = open(filename, "r")
    lines = f.readlines()

    data = []
    start = ""
    
    if lines == 0:
        print("No packets received")
    else:  
        for line in lines:
            if line.startswith("seq"):
                data.append(line.strip())
            elif line.startswith("start"):
                start = line.strip()

        start_split = start.split()
        packets_sent = start_split[2]
    
        return data, packets_sent

    return 0

# Packet class to store all packet data
class Packet():
    def __init__(self, seq, time, rtt):
        self.seq = seq
        self.time = time
        self.rtt = rtt
    

# Processes the input lines and retrieves the packet information
def getPacketInfo(data):

    packets = []

    for x in data:

        x_split = x.split()
        x_seq = x_split[1]
        x_time = x_split[3]
        x_rtt = x_split[5]
        temp_packet = Packet(x_seq, x_time, x_rtt)
        packets.append(temp_packet)

    return packets


# Creates the bins, for the box plot diagrams
def createBins(rtt, seq, total_sent, bin_size):

    total_bins = int(int(total_sent) / bin_size)

    bins_data = []
    x = 0

    for bin_index in range(0, total_bins):

        data = []
 
        while (int(seq[x]) < (bin_index * bin_size) + bin_size):
            data.append(rtt[x])
            x += 1

        bins_data.append(data)

    return bins_data


def getDataRate(data, data_big):

    large_packet_size = 1400
    small_packet_size = 16

    b = (large_packet_size - small_packet_size) * 8 

    T_x = (b / (10**9))

    rtt = []
    rtt_big = []

    for x in data:
        rtt.append(float(x.rtt))

    for x_big in data_big:
        rtt_big.append(float(x_big.rtt))


    T_d_path = ((np.median(rtt_big) - np.median(rtt))/2)/1000

    T_p = T_d_path - T_x

    r = (b / T_p)

    return np.round(r / (10**6), decimals = 2)
