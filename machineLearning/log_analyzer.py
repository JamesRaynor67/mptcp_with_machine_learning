import csv
import matplotlib.pyplot as plt
import seaborn as sns
import sys
import os
import pandas as pd
import numpy as np
import warnings
from optparse import OptionParser

from log_helper_mptcp_subflow_id import MpTcpSubflows
from log_monitor_time_bytes import AnalyzeSentBytes
from log_monitor_time_bytes import AnalyzeReceivedBytes
from log_monitor_time_bytes import AnalyzeBytes
from log_monitor_time_sendingRate import AnalyzeMonitorSendingRate
from log_monitor_time_sendingRate import  AnalyzeMonitorSendingRateUtilization
from log_time_tcb import AnalyzeClientRtt
from log_time_tcb import AnalyzeClientCwnd
from log_time_tcb import AnalyzeClientRwnd
from log_time_tcb import AnalyzeClientUnAck
from log_time_tcb import AnalyzeMetaSocket

warnings.filterwarnings("error")
g_resultRecord = {}

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

def proprocess_meta_socket_data(file_path):
    record = []
    with open(file_path, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',')
        next(spamreader)
        for row in spamreader:
            timestamp = (int(row[0])*1.0)/1e6
            lastAckedSeq = row[1]
            highTxMark = row[2]
            availableTxBuffer = row[3]
            nextTxSeq = row[4]
            totalCwnd = row[5]
            unAck = row[6]
            record.append([timestamp, lastAckedSeq, highTxMark, availableTxBuffer, nextTxSeq, totalCwnd, unAck])

    columns = ['Timestamp', 'LastAckedSeq', 'HighTxMark', 'AvailableTxBuffer', 'NextTxSeq', 'TotalCwnd', 'UnAck']
    meta_socket_records = pd.DataFrame(record, columns=columns)

    return meta_socket_records


def proprocess_cWnd_data(file_path):
    record = []
    valid = [False, False]
    with open(file_path, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',')
        next(spamreader)
        for row in spamreader:
            timestamp = (int(row[0])*1.0)/1e6

            if valid[0] is False:
                valid[0] = True if int(row[1]) != -1 else False
            if valid[1] is False:
                valid[1] = True if int(row[2]) != -1 else False

            cWnd0 = int(row[1]) if valid[0] is True else 0
            cWnd1 = int(row[2]) if valid[1] is True else 0

            record.append([timestamp, cWnd0, cWnd1])

    columns = ['Timestamp','cWnd0','cWnd1']
    cWnd_records = pd.DataFrame(record, columns=columns)

    return cWnd_records

def proprocess_rWnd_data(file_path):
    record = []
    valid = [False, False]
    with open(file_path, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',')
        next(spamreader)
        for row in spamreader:
            timestamp = (int(row[0])*1.0)/1e6

            if valid[0] is False:
                valid[0] = True if int(row[1]) != -1 else False
            if valid[1] is False:
                valid[1] = True if int(row[2]) != -1 else False

            rWnd0 = int(row[1]) if valid[0] is True else 0
            rWnd1 = int(row[2]) if valid[1] is True else 0

            record.append([timestamp, rWnd0, rWnd1])

    columns = ['Timestamp','rWnd0','rWnd1']
    rWnd_records = pd.DataFrame(record, columns=columns)

    return rWnd_records

def proprocess_unAck_data(file_path):
    record = []
    valid = [False, False]
    with open(file_path, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',')
        next(spamreader)
        for row in spamreader:
            timestamp = (int(row[0])*1.0)/1e6

            if valid[0] is False:
                valid[0] = True if int(row[1]) != -1 else False
            if valid[1] is False:
                valid[1] = True if int(row[2]) != -1 else False

            unAck0 = int(row[1]) if valid[0] is True else 0
            unAck1 = int(row[2]) if valid[1] is True else 0

            record.append([timestamp, unAck0, unAck1])

    columns = ['Timestamp','unAck0','unAck1']
    unAck_records = pd.DataFrame(record, columns=columns)

    return unAck_records

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

def proprocess_clientAvailableTxBuffer_data(file_path):
    record = []
    valid = [False, False]
    with open(file_path, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',')
        next(spamreader)
        for row in spamreader:
            timestamp = (int(row[0])*1.0)/1e6

            if valid[0] is False:
                valid[0] = True if int(row[1]) != -1 else False
            if valid[1] is False:
                valid[1] = True if int(row[2]) != -1 else False

            availableTxBuffer0 = int(row[1]) if valid[0] is True else 0
            availableTxBuffer1 = int(row[2]) if valid[1] is True else 0

            record.append([timestamp, availableTxBuffer0, availableTxBuffer1])

    columns = ['Timestamp','availableTxBuffer0','availableTxBuffer1']
    availableTxBuffer_records = pd.DataFrame(record, columns=columns)

    return availableTxBuffer_records

def analyze_client_end_node(file_path):
    record = []
    tmp_client_sent_count, tmp_client_rcv_count = 0, 0
    # '/home/hong/workspace/mptcp/ns3/mptcp_output/mptcp_client'
    with open(file_path, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',')
        next(spamreader)
        for row in spamreader:
            if int(row[1]) == 1: # not receive record
                tmp_client_sent_count += 1
                timestamp = int(row[0])/1e9
                subflowId = int(row[3])
                seqnum = int(row[4])
                if subflowId >= 0: # for non-mptcp packet, subflowId will be -1
                    record.append([timestamp, subflowId, seqnum])
            else:
                tmp_client_rcv_count += 1

    print 'client sent count: ', tmp_client_sent_count, 'client receive count: ', tmp_client_rcv_count
    record.sort(key=lambda ele:ele[0])
    x, y = [[],[]], [[],[]]
    for row in record:
        # subflow id is from 0 to n-1
        x[row[1]].append(row[0])
        y[row[1]].append(row[2])

    global g_resultRecord
    g_resultRecord["Subflow0_Client_Sent"] = y[0][-1]
    g_resultRecord["Subflow1_Client_Sent"] = y[1][-1]

    subflow_1, = sns.plt.plot(x[0], y[0], 'b-')
    subflow_2, = sns.plt.plot(x[1], y[1], 'r-')
    sns.plt.legend([subflow_1, subflow_2], ['client side subflow 1', 'client side subflow 2'], loc='upper left')
    sns.plt.title('Client Side Time-Seqence number, Max SeqSum == ' + str(sum([row[-1] for row in y])))
    sns.plt.xlabel('Time / s', fontsize = 14, color = 'black')
    sns.plt.ylabel('Seqence number', fontsize = 14, color = 'black')

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
    subflow_1, = plt.plot(x[0], y[0], 'b-')
    subflow_2, = plt.plot(x[1], y[1], 'r-')
    
    global g_resultRecord
    g_resultRecord["Subflow0_Server_Rcv"] = y[0][-1]
    g_resultRecord["Subflow1_Server_Rcv"] = y[1][-1]
    
    sns.plt.legend([subflow_1, subflow_2], ['server side subflow 1', 'server side subflow 2'], loc='upper left')
    sns.plt.title('Server Side Time-Seqence number, Max SeqSum == ' + str(sum([row[-1] for row in y])))
    sns.plt.xlabel('Time / s', fontsize = 14, color = 'black')
    sns.plt.ylabel('Seqence number', fontsize = 14, color = 'black')

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

def recordResultToCsv():
    global g_resultRecord
    if g_resultRecord['Experiment'] != None and g_resultRecord['Scheduler'] != None:
        print 'Recording to csv: ' + g_resultRecord['Filename']
        record = []
        record.append(g_resultRecord['Experiment'])
        record.append(g_resultRecord['Scheduler'])
        record.append(g_resultRecord['Subflow0_Client_Sent'])
        record.append(g_resultRecord['Subflow1_Client_Sent'])
        record.append(g_resultRecord['Subflow0_Server_Rcv'])
        record.append(g_resultRecord['Subflow1_Server_Rcv'])
        with open(g_resultRecord['Filename'],'a') as f:
            writer = csv.writer(f)
            writer.writerow(record)
    else:
        print "Not recording result: ", g_resultRecord
    g_resultRecord = {}

if __name__ == '__main__':

    parser = OptionParser()
    parser.add_option("-f", "--filename", dest="Filename", default="/home/hong/result_figure/statistic.csv", help="write report to FILE", metavar="FILE")
    parser.add_option("-e", "--experiment", dest="Experiment", default=None, help="The name(index) of experiment. E.g. \"Exp_1\"")
    parser.add_option("-s", "--scheduler", dest="Scheduler", default=None, help="The name of sechduler. E.g. \"RR\"")
    parser.add_option("-n", "--episodeNum", dest="EpisodeNum", default=None, help="The number of episode to analyze")
    parser.add_option("-b", "--linkBBandwidth", dest="LinkBBW", default=None, help="The bandwidth of link B")
    parser.add_option("-c", "--linkCBandwidth", dest="LinkCBW", default=None, help="The bandwidth of link C")
    parser.add_option("-d", "--dirPath", dest="DirPath", default=None, help="The dir path to store figures and relative data")
    (options, args) = parser.parse_args()
    g_resultRecord['Filename'] = options.Filename
    g_resultRecord['Experiment'] = options.Experiment
    g_resultRecord['Scheduler'] = options.Scheduler
    g_resultRecord['DirPath'] = options.DirPath
    assert os.path.isdir(options.DirPath) is True

    episode_num = int(options.EpisodeNum)
    sns.plt.figure(figsize=(16*2, 9*2))

    sns.plt.subplot(4,2,1)
    analyze_client_end_node('/home/hong/workspace/mptcp/ns3/rl_training_data/' + options.EpisodeNum + '_mptcp_client')
    
    sns.plt.subplot(4,2,2)
    analyze_server_end_point('/home/hong/workspace/mptcp/ns3/rl_training_data/' + options.EpisodeNum + '_mptcp_server')
    
    sns.plt.subplot(4,2,3)
    AnalyzeBytes('/home/hong/workspace/mptcp/ns3/rl_training_data/' + options.EpisodeNum + '_mptcp_client', '/home/hong/workspace/mptcp/ns3/rl_training_data/' + options.EpisodeNum + '_mptcp_server')
    
    # AnalyzeSentBytes('/home/hong/workspace/mptcp/ns3/rl_training_data/' + options.EpisodeNum + '_mptcp_client')
    # AnalyzeReceivedBytes('/home/hong/workspace/mptcp/ns3/rl_training_data/' + options.EpisodeNum + '_mptcp_server')
    
    sns.plt.subplot(4,2,4)
    monitor_records = preprocess_monitor_data('/home/hong/workspace/mptcp/ns3/rl_training_data/' + options.EpisodeNum + '_mptcp_monitor')
    subflow_rates, smoothed_subflow_rates = AnalyzeMonitorSendingRate(monitor_records)
    
    sns.plt.subplot(4,2,5)
    AnalyzeMonitorSendingRateUtilization(subflow_rates, smoothed_subflow_rates, [int(options.LinkBBW[:-4]), int(options.LinkCBW[:-4])])
    
    sns.plt.subplot(4,2,6)
    rtt_records = proprocess_rtt_data('/home/hong/workspace/mptcp/ns3/rl_training_data/' + options.EpisodeNum + '_client_rtt')
    AnalyzeClientRtt(rtt_records)

    sns.plt.subplot(4,2,7)
    cWnd_records = proprocess_cWnd_data('/home/hong/workspace/mptcp/ns3/rl_training_data/' + options.EpisodeNum + '_client_cWnd')
    AnalyzeClientCwnd(cWnd_records)
    
    # sns.plt.subplot(4,2,8)
    # rWnd_records = proprocess_rWnd_data('/home/hong/workspace/mptcp/ns3/rl_training_data/' + options.EpisodeNum + '_client_rWnd')
    # AnalyzeClientRwnd(rWnd_records)

    sns.plt.subplot(4,2,8)
    unAck_records = proprocess_unAck_data('/home/hong/workspace/mptcp/ns3/rl_training_data/' + options.EpisodeNum + '_client_unAck')
    AnalyzeClientUnAck(unAck_records)

    # sns.plt.savefig("/home/hong/result_figure/0_static_well_designed/I_rr.png", dpi = 150, bbox_inches='tight')
    sns.plt.savefig(os.path.join(options.DirPath, options.Experiment + "_" + options.Scheduler + "_" + options.EpisodeNum + ".png"), dpi = 150, bbox_inches='tight')
    # sns.plt.savefig("/home/hong/result_figure/0_static_well_designed/Z1_rr.png", dpi = 150, bbox_inches='tight')
    sns.plt.close()

    sns.plt.figure(figsize=(16*2, 9*2))
    meta_socket_records = proprocess_meta_socket_data('/home/hong/workspace/mptcp/ns3/rl_training_data/' + options.EpisodeNum + '_meta_socket')
    clientAvailableTxBuffer_records = proprocess_clientAvailableTxBuffer_data('/home/hong/workspace/mptcp/ns3/rl_training_data/' + options.EpisodeNum + '_client_txBufferSize')
    AnalyzeMetaSocket(meta_socket_records, clientAvailableTxBuffer_records)
    sns.plt.savefig(os.path.join(options.DirPath, options.Experiment + "_" + options.Scheduler + '_meta_' + options.EpisodeNum + ".png"), dpi = 150, bbox_inches='tight')
    sns.plt.close()

    recordResultToCsv()

    # analyze_reward('/home/hong/workspace/mptcp/ns3/rl_training_data/' + options.EpisodeNum + '_calculate_reward')
