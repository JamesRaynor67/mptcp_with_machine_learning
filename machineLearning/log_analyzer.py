import csv
import matplotlib.pyplot as plt
import seaborn as sns
import sys
import os
import pandas as pd
import warnings

from log_helper_mptcp_subflow_id import MpTcpSubflows
from log_monitor_time_sentBytes import AnalyzeMonitorSentBytes
from log_monitor_time_sendingRate import AnalyzeMonitorSendingRate
from log_monitor_time_sendingRate import  AnalyzeMonitorSendingRateUtilization
from log_time_rtt import AnalyzeClientRtt

warnings.filterwarnings("error")

def preprocess_monitor_data(file_path):
    record = []
    with open(file_path, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',')
        next(spamreader)
        for row in spamreader:
            timestamp = (int(row[0])*1.0)/1e9
            flowId = int(row[1])
            From = row[2]
            To = row[3]
            TxPacket = int(row[4])
            TxBytes = int(row[5]) # - int(row[4]) * 52 # minus the size of header (most header is of size 20 (tcp header) + 32 (22 for mptcp DSS and 10 for TS) Bytes)
            RxPacket = int(row[6])
            RxBytes = int(row[7]) # - int(row[6]) * 52 # minus the size of header (most header is of size 20 (tcp header) + 32 (22 for mptcp DSS and 10 for TS) Bytes)
            delaySum = float(row[8][1:-2])/1e9
            jitterSum = float(row[9][1:-2])/1e9
            lostPackets = int(row[10])

            record.append([timestamp, flowId, From, To, TxPacket, TxBytes, RxPacket, RxBytes, delaySum, jitterSum, lostPackets])
            MpTcpSubflows.updateSubflowId(From, To, flowId)

    record.sort(key=lambda ele:ele[0])

    print 'mptcp subflow ids: ', MpTcpSubflows.getSubflowList()
    assert -1 not in MpTcpSubflows.getSubflowList()
    for row in record:
        row.append(MpTcpSubflows.getSubflowId(row[1])) # append -1 if it is not a flowId of subflow

    columns = ['Timestamp','FlowId','From','To','TxPackets','TxBytes','RxPackets','RxBytes','DelaySum','JitterSum','LostPacketSum','SubflowId']
    monitor_records = pd.DataFrame(record, columns=columns)

    return monitor_records

def proprocess_rtt_data(file_path):
    record = []
    valid = [False, False]
    with open(file_path, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',')
        next(spamreader)
        for row in spamreader:
            timestamp = (int(row[0])*1.0)/1e6

            if valid[0] is False:
                valid[0] = True if (int(row[1]) != -1 and int(row[1]) != 1000000) else False
            if valid[1] is False:
                valid[1] = True if (int(row[2]) != -1 and int(row[2]) != 1000000) else False

            rtt0 = int(row[1]) if valid[0] is True else 0
            rtt1 = int(row[2]) if valid[1] is True else 0

            record.append([timestamp, rtt0, rtt1])

    columns = ['Timestamp','Rtt0','Rtt1']
    rtt_records = pd.DataFrame(record, columns=columns)

    return rtt_records

def analyze_application(file_path):
    record = []
    with open(file_path, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',')
        next(spamreader)
        total_psize = 0
        for row in spamreader:
            if int(row[1]) == 1: # not receive record
                timestamp = int(row[0])/1e9
                total_psize += int(row[7])
                record.append([timestamp, total_psize])

    record.sort(key=lambda ele:ele[0])
    x, y = [], []
    for pair in record:
        x.append(pair[0])
        y.append(pair[1])
    sent_packet_size, = sns.plt.plot(x, y, 'go')
    sns.plt.legend([sent_packet_size], ['sent packet size'], loc='upper left')
    sns.plt.title('Time-Sent packet size')
    sns.plt.xlabel('Time / s', fontsize = 14, color = 'black')
    sns.plt.ylabel('Sent packet size / Byte', fontsize = 14, color = 'black')
    print 'server send total: ', y[-1], ' Bytes' 

def analyze_client_end_node(file_path):
    record = []
    # '/home/hong/workspace/mptcp/ns3/mptcp_output/mptcp_client'
    with open(file_path, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',')
        next(spamreader)
        for row in spamreader:
            if int(row[1]) == 1: # not receive record
                timestamp = int(row[0])/1e9
                subflowId = int(row[3])
                seqnum = int(row[4])
                if subflowId >= 0: # for non-mptcp packet, subflowId will be -1
                    record.append([timestamp, subflowId, seqnum])

    record.sort(key=lambda ele:ele[0])
    x, y = [[],[]], [[],[]]
    for row in record:
        # subflow id is from 0 to n-1
        x[row[1]].append(row[0])
        y[row[1]].append(row[2])
    print len(y),len(y[0]),len(y[1])
    subflow_1, = plt.plot(x[0], y[0], 'ro')
    subflow_2, = plt.plot(x[1], y[1], 'bo')
    sns.plt.legend([subflow_1, subflow_2], ['client side subflow 1', 'client side subflow 2'], loc='upper left')
    sns.plt.title('Client Side Time-Seqence number, Max SeqSum == ' + str(sum([row[-1] for row in y])))
    sns.plt.xlabel('Time / s', fontsize = 14, color = 'black')
    sns.plt.ylabel('Seqence number', fontsize = 14, color = 'black')
    writeToCsv(sentBytes = sum([row[-1] for row in y]))

def analyze_server_end_point(file_path):
    record = []
    # '/home/hong/workspace/mptcp/ns3/mptcp_output/mptcp_client'
    with open(file_path, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',')
        next(spamreader)
        for row in spamreader:
            if int(row[1]) == 0: # not send record
                timestamp = int(row[0])/1e9
                subflowId = int(row[3])
                seqnum = int(row[4])
                if subflowId >= 0: # for non-mptcp packet, subflowId will be -1
                    record.append([timestamp, subflowId, seqnum])

    record.sort(key=lambda ele:ele[0])
    x, y = [[],[]], [[],[]]
    for row in record:
        # subflow id is from 0 to n-1
        x[row[1]].append(row[0])
        y[row[1]].append(row[2])
    subflow_1, = plt.plot(x[0], y[0], 'ro')
    subflow_2, = plt.plot(x[1], y[1], 'bo')
    sns.plt.legend([subflow_1, subflow_2], ['server side subflow 1', 'server side subflow 2'], loc='upper left')
    sns.plt.title('Server Side Time-Seqence number, Max SeqSum == ' + str(sum([row[-1] for row in y])))
    sns.plt.xlabel('Time / s', fontsize = 14, color = 'black')
    sns.plt.ylabel('Seqence number', fontsize = 14, color = 'black')
    writeToCsv(receivedBytes = sum([row[-1] for row in y]))

def analyze_reward(file_path):
    record = []
    # '/home/hong/workspace/mptcp/ns3/mptcp_output/mptcp_client'
    with open(file_path, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',')
        next(spamreader)
        for row in spamreader:
            timestamp = int(row[0])/1e9
            reward = int(row[1])
            record.append([timestamp, reward])

    record.sort(key=lambda ele:ele[0])
    x, y = [], []
    for pair in record:
        x.append(pair[0])
        y.append(pair[1])
    reward_plt, = plt.plot(x, y, 'k-')
    sns.plt.legend([reward_plt], ['reward'], loc='best')
    sns.plt.title('Time-Reward')
    sns.plt.xlabel('Time / s', fontsize = 14, color = 'black')
    sns.plt.ylabel('Reward', fontsize = 14, color = 'black')

def writeToCsv(sentBytes = None, receivedBytes = None):
    if(len(sys.argv) >= 4 and sys.argv[2] == 'true'):
        path = sys.argv[3]
        scheduler = sys.argv[4]

        assert os.path.isfile(path) 
        new_rows = []
        with open(path, 'rb') as csv_file:
            r = csv.reader(csv_file)
            row_num = 0
            for row in r:
                if row_num == 0 and scheduler == 'rr' and sentBytes is not None:
                    row.append(sentBytes)
                elif row_num == 1 and scheduler == 'rtt' and sentBytes is not None:
                    row.append(sentBytes)
                elif row_num == 2 and scheduler == 'rf' and sentBytes is not None:
                    row.append(sentBytes)
                elif row_num == 3 and scheduler == 'ldbp' and sentBytes is not None:
                    row.append(sentBytes)
                elif row_num == 4 and scheduler == 'rr' and receivedBytes is not None:
                    row.append(receivedBytes)
                elif row_num == 5 and scheduler == 'rtt' and receivedBytes is not None:
                    row.append(receivedBytes)
                elif row_num == 6 and scheduler == 'rf' and receivedBytes is not None:
                    row.append(receivedBytes)
                elif row_num == 7 and scheduler == 'ldbp' and receivedBytes is not None:
                    row.append(receivedBytes)
                row_num += 1
                new_rows.append(row)

        with open(path, 'wb') as csv_file:
            w = csv.writer(csv_file)
            w.writerows(new_rows)
                
if __name__ == '__main__':

    # batch_num = int(sys.argv[1])
    # sns.plt.figure(figsize=(16*2, 9*2))
    # sns.plt.subplot(3,1,1)
    # analyze_application('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(batch_num) + '_mptcp_client')
    # sns.plt.subplot(3,1,2)
    # analyze_client_end_node('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(batch_num) + '_mptcp_client')
    # sns.plt.subplot(3,1,3)
    # analyze_server_end_point('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(batch_num) + '_mptcp_server')
    # sns.plt.savefig("/home/hong/result_figure/tmp0.png", dpi = 150, bbox_inches='tight')
    # sns.plt.close()

    # monitor_records = preprocess_monitor_data('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(batch_num) + '_mptcp_monitor')
    # sns.plt.figure(figsize=(16*2, 9*2))
    # sns.plt.subplot(3,1,1)
    # AnalyzeMonitorSentBytes(monitor_records)
    # sns.plt.subplot(3,1,2)
    # subflow_rates, smoothed_subflow_rates = AnalyzeMonitorSendingRate(monitor_records)
    # sns.plt.subplot(3,1,3)
    # AnalyzeMonitorSendingRateUtilization(subflow_rates, smoothed_subflow_rates, [100,100])
    # sns.plt.savefig("/home/hong/result_figure/tmp1.png", dpi = 150, bbox_inches='tight')
    # sns.plt.close()

    # sns.plt.figure(figsize=(16*2, 9*2))
    # rtt_records = proprocess_rtt_data('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(batch_num) + '_client_rtt')
    # AnalyzeClientRtt(rtt_records)
    # sns.plt.savefig("/home/hong/result_figure/tmp2.png", dpi = 150, bbox_inches='tight')
    # sns.plt.close()


    batch_num = int(sys.argv[1])
    sns.plt.figure(figsize=(16*2, 9*2))
    sns.plt.subplot(3,2,1)
    analyze_application('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(batch_num) + '_mptcp_client')
    sns.plt.subplot(3,2,2)
    analyze_client_end_node('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(batch_num) + '_mptcp_client')
    sns.plt.subplot(3,2,3)
    analyze_server_end_point('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(batch_num) + '_mptcp_server')

    monitor_records = preprocess_monitor_data('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(batch_num) + '_mptcp_monitor')
    sns.plt.subplot(3,2,4)
    AnalyzeMonitorSentBytes(monitor_records)
    sns.plt.subplot(3,2,5)
    subflow_rates, smoothed_subflow_rates = AnalyzeMonitorSendingRate(monitor_records)
    sns.plt.subplot(3,2,6)
    AnalyzeMonitorSendingRateUtilization(subflow_rates, smoothed_subflow_rates, [100,100])
    sns.plt.savefig("/home/hong/result_figure/tmp3.png", dpi = 150, bbox_inches='tight')
    sns.plt.close()

    sns.plt.figure(figsize=(16*2, 9*2))
    rtt_records = proprocess_rtt_data('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(batch_num) + '_client_rtt')
    AnalyzeClientRtt(rtt_records)
    sns.plt.savefig("/home/hong/result_figure/tmp4.png", dpi = 150, bbox_inches='tight')
    sns.plt.close()







    # # print sys.argv[1] ,sys.argv[2], sys.argv[3], sys.argv[4]

    # analyze_reward('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(batch_num) + '_calculate_reward')
