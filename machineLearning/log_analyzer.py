import csv
import matplotlib.pyplot as plt

def plot_by_list(record):
    x, y = [], []
    for pair in record:
        x.append(pair[0])
        y.append(pair[1])
    plt.plot(x, y, 'r')
    plt.title('Time-Packet size')
    plt.xlabel('Time / s', fontsize = 14, color = 'black')
    plt.ylabel('Packet size / Byte', fontsize = 14, color = 'black')
    plt.show()

if __name__ == '__main__':
    record = []
    with open('/home/hong/workspace/mptcp/ns3/mptcp_output/mptcp_client', 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',')
        next(spamreader)
        for row in spamreader:
            if int(row[1]) == 0: # not send record
                timestamp = int(row[0])/10e8
                psize = int(row[7])
                record.append([timestamp, psize])

    record.sort(key=lambda ele:ele[0])
    plot_by_list(record)
