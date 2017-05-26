import csv
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
from log_helper_mptcp_subflow_id import MpTcpSubflows

def AnalyzeSentBytes(file_path):
    total_sent = []
    subflow0_sent = []
    subflow2_sent = []
    with open(file_path, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',')
        next(spamreader)
        total_psize, subflow0_psize, subflow2_psize = 0, 0, 0
        for row in spamreader:
            if int(row[1]) == 1: # not receive record
                timestamp = int(row[0])/1e9
                total_psize += int(row[7])
                total_sent.append([timestamp, total_psize])
                if int(row[3]) == 0:
                    subflow0_psize += int(row[7])
                    subflow0_sent.append([timestamp, subflow0_psize])
                elif int(row[3]) == 1:
                    subflow2_psize += int(row[7])
                    subflow2_sent.append([timestamp, subflow2_psize])
                else:
                    assert False # Should not go to here if only 2 subflows

    total_sent.sort(key=lambda ele:ele[0])
    subflow0_sent.sort(key=lambda ele:ele[0])
    subflow2_sent.sort(key=lambda ele:ele[0])

    total_sent = np.array(total_sent)
    subflow0_sent = np.array(subflow0_sent)
    subflow2_sent = np.array(subflow2_sent)

    c_s_subflow_total, = sns.plt.plot(total_sent[:,0], total_sent[:,1])
    c_s_subflow_0, = sns.plt.plot(subflow0_sent[:,0], subflow0_sent[:,1])
    c_s_subflow_1, = sns.plt.plot(subflow2_sent[:,0], subflow2_sent[:,1])
    sns.plt.legend([c_s_subflow_total, c_s_subflow_0, c_s_subflow_1], ['Total sent bytes', 'Subflow 1 sent bytes', 'Subflow 2 sent bytes'], loc='best')
    sns.plt.title('Time-Sent Data Size')
    sns.plt.xlabel('Time / s', fontsize = 14, color = 'black')
    sns.plt.ylabel('Sent Data Size / Bytes', fontsize = 14, color = 'black')


def AnalyzeReceivedBytes(file_path):
    total_received = []
    subflow0_received = []
    subflow2_received = []
    with open(file_path, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',')
        next(spamreader)
        total_psize, subflow0_psize, subflow2_psize = 0, 0, 0
        for row in spamreader:
            if int(row[1]) == 0: # not send record
                timestamp = int(row[0])/1e9
                total_psize += int(row[7])
                total_received.append([timestamp, total_psize])
                if int(row[3]) == 0:
                    subflow0_psize += int(row[7])
                    subflow0_received.append([timestamp, subflow0_psize])
                elif int(row[3]) == 1:
                    subflow2_psize += int(row[7])
                    subflow2_received.append([timestamp, subflow2_psize])
                else:
                    assert False # Should not go to here if only 2 subflows

    total_received.sort(key=lambda ele:ele[0])
    subflow0_received.sort(key=lambda ele:ele[0])
    subflow2_received.sort(key=lambda ele:ele[0])

    total_received = np.array(total_received)
    subflow0_received = np.array(subflow0_received)
    subflow2_received = np.array(subflow2_received)

    c_s_subflow_total, = sns.plt.plot(total_received[:,0], total_received[:,1])
    c_s_subflow_0, = sns.plt.plot(subflow0_received[:,0], subflow0_received[:,1])
    c_s_subflow_1, = sns.plt.plot(subflow2_received[:,0], subflow2_received[:,1])
    sns.plt.legend([c_s_subflow_total, c_s_subflow_0, c_s_subflow_1], ['Total received bytes', 'Subflow 1 received bytes', 'Subflow 2 received bytes'], loc='best')
    sns.plt.title('Time-Received Data Size')
    sns.plt.xlabel('Time / s', fontsize = 14, color = 'black')
    sns.plt.ylabel('Received Data Size / Bytes', fontsize = 14, color = 'black')

def AnalyzeBytes(client_file_path, server_file_path):
    total_sent = []
    subflow0_sent = []
    subflow2_sent = []
    with open(client_file_path, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',')
        next(spamreader)
        total_psize, subflow0_psize, subflow2_psize = 0, 0, 0
        for row in spamreader:
            if int(row[1]) == 1: # not receive record
                timestamp = int(row[0])/1e9
                total_psize += int(row[7])
                total_sent.append([timestamp, total_psize])
                if int(row[3]) == 0:
                    subflow0_psize += int(row[7])
                    subflow0_sent.append([timestamp, subflow0_psize])
                elif int(row[3]) == 1:
                    subflow2_psize += int(row[7])
                    subflow2_sent.append([timestamp, subflow2_psize])
                else:
                    assert False # Should not go to here if only 2 subflows

    total_sent.sort(key=lambda ele:ele[0])
    subflow0_sent.sort(key=lambda ele:ele[0])
    subflow2_sent.sort(key=lambda ele:ele[0])

    total_sent = np.array(total_sent)
    subflow0_sent = np.array(subflow0_sent)
    subflow2_sent = np.array(subflow2_sent)

    c_s_subflow_total_sent, = sns.plt.plot(total_sent[:,0], total_sent[:,1])
    c_s_subflow_0_sent, = sns.plt.plot(subflow0_sent[:,0], subflow0_sent[:,1])
    c_s_subflow_1_sent, = sns.plt.plot(subflow2_sent[:,0], subflow2_sent[:,1])

    total_received = []
    subflow0_received = []
    subflow2_received = []
    with open(server_file_path, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',')
        next(spamreader)
        total_psize, subflow0_psize, subflow2_psize = 0, 0, 0
        for row in spamreader:
            if int(row[1]) == 0: # not send record
                timestamp = int(row[0])/1e9
                total_psize += int(row[7])
                total_received.append([timestamp, total_psize])
                if int(row[3]) == 0:
                    subflow0_psize += int(row[7])
                    subflow0_received.append([timestamp, subflow0_psize])
                elif int(row[3]) == 1:
                    subflow2_psize += int(row[7])
                    subflow2_received.append([timestamp, subflow2_psize])
                else:
                    assert False # Should not go to here if only 2 subflows

    total_received.sort(key=lambda ele:ele[0])
    subflow0_received.sort(key=lambda ele:ele[0])
    subflow2_received.sort(key=lambda ele:ele[0])

    total_received = np.array(total_received)
    subflow0_received = np.array(subflow0_received)
    subflow2_received = np.array(subflow2_received)

    c_s_subflow_total_received, = sns.plt.plot(total_received[:,0], total_received[:,1])
    c_s_subflow_0_received, = sns.plt.plot(subflow0_received[:,0], subflow0_received[:,1])
    c_s_subflow_1_received, = sns.plt.plot(subflow2_received[:,0], subflow2_received[:,1])
    sns.plt.legend([c_s_subflow_total_sent, c_s_subflow_0_sent, c_s_subflow_1_sent, c_s_subflow_total_received, c_s_subflow_0_received, c_s_subflow_1_received], \
                    ['Total sent bytes', 'Subflow 1 sent bytes', 'Subflow 2 sent bytes', 'Total received bytes', 'Subflow 1 received bytes', 'Subflow 2 received bytes'], loc='best')
    sns.plt.title('Time-Sent/Received Data Size')
    sns.plt.xlabel('Time / s', fontsize = 14, color = 'black')
    sns.plt.ylabel('Data Size / Bytes', fontsize = 14, color = 'black')
