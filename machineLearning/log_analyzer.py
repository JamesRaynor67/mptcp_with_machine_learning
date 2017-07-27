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
from log_monitor_time_sendingRate import AnalyzeMonitorSendingRateUtilization
from log_monitor_time_sendingRate import AnalyzeMonitorThroughput
from log_time_tcb import AnalyzeClientRtt
from log_time_tcb import AnalyzeClientCwnd
from log_time_tcb import AnalyzeClientRwnd
from log_time_tcb import AnalyzeClientUnAck
from log_time_tcb import AnalyzeMetaSocket
from log_time_queue import AnalyzeQueueLength

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
    meta_socket_records.to_csv(os.path.join(g_resultRecord['DirPath'], "metaSocketData_"+g_resultRecord['Experiment']+"_"+g_resultRecord['Scheduler']+"_"+g_resultRecord['EpisodeNum']+".csv"))
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
    availableTxBuffer_records.to_csv(os.path.join(g_resultRecord['DirPath'], "subflowTxBufferData_"+g_resultRecord['Experiment']+"_"+g_resultRecord['Scheduler']+"_"+g_resultRecord['EpisodeNum']+".csv"))
    return availableTxBuffer_records

def proprocess_schedulerId_data(file_path):
    record = []
    with open(file_path, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',')
        next(spamreader)
        for row in spamreader:
            timestamp = (int(row[0])*1.0)/1e6
            schedulerId = int(row[1])
            record.append([timestamp, schedulerId])

    columns = ['Timestamp','schedulerId']
    scheduler_records = pd.DataFrame(record, columns=columns)
    scheduler_records.to_csv(os.path.join(g_resultRecord['DirPath'], "scheduler_"+g_resultRecord['Experiment']+"_"+g_resultRecord['Scheduler']+"_"+g_resultRecord['EpisodeNum']+".csv"))
    return scheduler_records

def analyze_server_client_seq_num(server_file_path, client_file_path, drop_file_path):
    client_tx_record = []
    with open(client_file_path, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',')
        next(spamreader)
        for row in spamreader:
            if int(row[1]) == 1: # not receive record
                timestamp = int(row[0])/1e9
                subflowId = int(row[3])
                seqnum = int(row[4])
                psize = int(row[6]) ######################################################### change 7 to 6
                if subflowId >= 0: # for non-mptcp packet, subflowId will be -1
                    client_tx_record.append([timestamp, subflowId, seqnum, psize])

    client_tx_record.sort(key=lambda ele:ele[0])
    x, y = [[],[]], [[],[]]
    for row in client_tx_record:
        # subflow id is from 0 to n-1
        x[row[1]].append(row[0])
        y[row[1]].append(row[2])

    global g_resultRecord
    g_resultRecord["Subflow0_Client_Sent"] = y[0][-1]
    g_resultRecord["Subflow1_Client_Sent"] = y[1][-1]

    subflow_1_client, = sns.plt.plot(x[0], y[0], 'b-.')
    subflow_2_client, = sns.plt.plot(x[1], y[1], 'r-.')
    client_max_seqSum = sum([row[-1] for row in y])

    ### Above is about client
    server_rx_record = []
    with open(server_file_path, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',')
        next(spamreader)
        for row in spamreader:
            if int(row[1]) == 0: # not send record
                timestamp = int(row[0])/1e9
                subflowId = int(row[3])
                seqnum = int(row[4])
                psize = int(row[6]) ######################################################### change 7 to 6
                if subflowId >= 0: # for non-mptcp packet, subflowId will be -1
                    server_rx_record.append([timestamp, subflowId, seqnum, psize])

    server_rx_record.sort(key=lambda ele:ele[0])
    x, y = [[],[]], [[],[]]
    for row in server_rx_record:
        # subflow id is from 0 to n-1
        x[row[1]].append(row[0])
        y[row[1]].append(row[2])
    subflow_1_server, = sns.plt.plot(x[0], y[0], 'b-')
    subflow_2_server, = sns.plt.plot(x[1], y[1], 'r-')
    server_max_seqSum = sum([row[-1] for row in y])
    
    g_resultRecord["Subflow0_Server_Rcv"] = y[0][-1]
    g_resultRecord["Subflow1_Server_Rcv"] = y[1][-1]
    ### Above is about server

    x, y = [], []
    with open(drop_file_path, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',')
        next(spamreader)
        for row in spamreader:
            timestamp = int(row[0])/1e9
            subflowId = int(row[1])
            seqnum = int(row[2])
            if subflowId != -1:
                x.append(timestamp)
                y.append(seqnum)
    drop_mark, = sns.plt.plot(x, y, 'k^')
    drop_count = len(x)
    
    sns.plt.legend([subflow_1_server, subflow_2_server, subflow_1_client, subflow_2_client, drop_mark], 
                    ['server side subflow 1', 'server side subflow 2', 'client side subflow 1', 'client side subflow 2', 'dropped packets'], loc='best')
    sns.plt.title('Server & Client Time-Seqence number, Max Server/Client SeqSum == ' + str(server_max_seqSum) + '/' + str(client_max_seqSum) + ', Dropped ' + str(drop_count) + ' packets')
    sns.plt.xlabel('Time / s', fontsize = 14, color = 'black')
    sns.plt.ylabel('Seqence number', fontsize = 14, color = 'black')
    return server_rx_record

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

def AnalyzeThroughput(server_rx_record):
    # server_rx_record.append([timestamp, subflowId, seqnum, psize])
    x, y = [[],[]], [[],[]]
    for row in server_rx_record:
        # subflow id is from 0 to n-1
        x[row[1]].append(row[0])
        y[row[1]].append(row[3])
    
    ts = np.arange(0.0, 180, 0.5) # NOTE: 180 is a magic number, because simulation time last for 180s
    sub0_throughPut = np.empty(ts.shape)
    sub1_throughPut = np.empty(ts.shape)
    total_throughPut = np.empty(ts.shape)
    sub0_headIndex, sub0_tailIndex, sub1_headIndex, sub1_tailIndex = 0, 0, 0, 0
    sub0_psizeInWindow, sub1_psizeInWindow, total_psizeInWindow = 0, 0, 0
    halfInterval = 0.5
    for i in range(ts.shape[0]):
        # print x[0][headIndex]
        # print sub0_ts[i]
        while sub0_headIndex < len(x[0]) and x[0][sub0_headIndex] < ts[i] + halfInterval:
            sub0_psizeInWindow += y[0][sub0_headIndex]
            sub0_headIndex += 1
        while sub0_tailIndex < len(x[0]) and x[0][sub0_tailIndex] <= ts[i] - halfInterval:
            sub0_psizeInWindow -= y[0][sub0_tailIndex]
            sub0_tailIndex += 1

        while sub1_headIndex < len(x[1]) and x[1][sub1_headIndex] < ts[i] + halfInterval:
            sub1_psizeInWindow += y[1][sub1_headIndex]
            sub1_headIndex += 1
        while sub1_tailIndex < len(x[1]) and x[1][sub1_tailIndex] <= ts[i] - halfInterval:
            sub1_psizeInWindow -= y[1][sub1_tailIndex]
            sub1_tailIndex += 1
        
        sub0_throughPut[i] = sub0_psizeInWindow / (halfInterval * 2.0)
        sub1_throughPut[i] = sub1_psizeInWindow / (halfInterval * 2.0)
        total_throughPut[i] = (sub0_psizeInWindow + sub1_psizeInWindow) / (halfInterval * 2.0)

    sub0_throughPut = sub0_throughPut * 8 / 1000.0 # convert bytes into Kbit
    sub1_throughPut = sub1_throughPut * 8 / 1000.0 # convert bytes into Kbit
    total_throughPut = total_throughPut * 8 / 1000.0

    global g_resultRecord
    throughputData = {"sub0": sub0_throughPut, "sub1":sub1_throughPut, "total":total_throughPut}
    df = pd.DataFrame.from_dict(throughputData)
    df.to_csv(os.path.join(g_resultRecord['DirPath'], "throughputData_"+g_resultRecord['Experiment']+"_"+g_resultRecord['Scheduler']+"_"+g_resultRecord['EpisodeNum']+".csv"))

    sub0_throughPut_plt, = plt.plot(ts, sub0_throughPut, 'b-')
    sub1_throughPut_plt, = plt.plot(ts, sub1_throughPut, 'r-')
    total_throughPut_plt, = plt.plot(ts, total_throughPut, 'k-')
    sns.plt.legend([sub0_throughPut_plt, sub1_throughPut_plt, total_throughPut_plt], ['Throughput of subflow 0', 'ThroughPut of subflow 0', 'Overall throughput'], loc='best')
    sns.plt.title('Time-Throughput')
    sns.plt.xlabel('Time / s', fontsize = 14, color = 'black')
    sns.plt.ylabel('Throughput / Kbps', fontsize = 14, color = 'black')

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

def extract_other_server_rx_record(other_server_file_path):
    other_server_rx_record = []
    with open(other_server_file_path, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',')
        next(spamreader)
        for row in spamreader:
            if int(row[1]) == 0: # not send record
                timestamp = int(row[0])/1e9
                seqnum = int(row[4])
                psize = int(row[6]) ######################################################### change 7 to 6
                other_server_rx_record.append([timestamp, seqnum, psize])
    other_server_rx_record.sort(key=lambda ele:ele[0])
    return other_server_rx_record

def WriteOtherServerThroughputToCsv(other_server_rx_record, other_server_id):
    x, y = [], []
    for row in other_server_rx_record:
        x.append(row[0])
        y.append(row[2])

    ts = np.arange(0.0, 180, 0.5) # NOTE: 180 is a magic number, because simulation time last for 180s
    throughPut = np.empty(ts.shape)
    headIndex, tailIndex, psizeInWindow = 0, 0, 0
    halfInterval = 0.5
    for i in range(ts.shape[0]):
        # print x[0][headIndex]
        # print sub0_ts[i]
        while headIndex < len(x) and x[headIndex] < ts[i] + halfInterval:
            psizeInWindow += y[headIndex]
            headIndex += 1
        while tailIndex < len(x) and x[tailIndex] <= ts[i] - halfInterval:
            psizeInWindow -= y[tailIndex]
            tailIndex += 1
        
        throughPut[i] = psizeInWindow / (halfInterval * 2.0)

    throughPut = throughPut * 8 / 1000.0 # convert bytes into Kbit

    global g_resultRecord
    throughputData = {"throughPut": throughPut}
    df = pd.DataFrame.from_dict(throughputData)
    assert other_server_id == 0 or other_server_id == 1
    df.to_csv(os.path.join(g_resultRecord['DirPath'], "other_server_" + str(other_server_id) + "_throughputData_"+g_resultRecord['Experiment']+"_"+g_resultRecord['Scheduler']+"_"+g_resultRecord['EpisodeNum']+".csv"))

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
    g_resultRecord['EpisodeNum'] = options.EpisodeNum
    assert os.path.isdir(options.DirPath) is True

    sns.plt.figure(figsize=(16*2, 9*2))

    sns.plt.subplot(4,2,1)
    server_file_path = '/home/hong/workspace/mptcp/ns3/rl_training_data/' + options.EpisodeNum + '_mptcp_server'
    client_file_path = '/home/hong/workspace/mptcp/ns3/rl_training_data/' + options.EpisodeNum + '_mptcp_client'
    drop_file_path = '/home/hong/workspace/mptcp/ns3/rl_training_data/' + options.EpisodeNum + '_mptcp_drops'
    server_rx_record = analyze_server_client_seq_num(server_file_path, client_file_path, drop_file_path)
    
    sns.plt.subplot(4,2,2)
    AnalyzeThroughput(server_rx_record)
    # sns.plt.subplot(4,2,2)
    # AnalyzeQueueLength('/home/hong/workspace/mptcp/ns3/rl_training_data/' + options.EpisodeNum + '_routers_queue_len')

    sns.plt.subplot(4,2,3)
    AnalyzeBytes('/home/hong/workspace/mptcp/ns3/rl_training_data/' + options.EpisodeNum + '_mptcp_client', '/home/hong/workspace/mptcp/ns3/rl_training_data/' + options.EpisodeNum + '_mptcp_server')
    
    AnalyzeSentBytes('/home/hong/workspace/mptcp/ns3/rl_training_data/' + options.EpisodeNum + '_mptcp_client')
    AnalyzeReceivedBytes('/home/hong/workspace/mptcp/ns3/rl_training_data/' + options.EpisodeNum + '_mptcp_server')
    
    sns.plt.subplot(4,2,4)
    monitor_records = preprocess_monitor_data('/home/hong/workspace/mptcp/ns3/rl_training_data/' + options.EpisodeNum + '_mptcp_monitor')
    subflow_rates = AnalyzeMonitorSendingRate(monitor_records)

    sns.plt.subplot(4,2,5)
    AnalyzeMonitorSendingRateUtilization(subflow_rates, [int(options.LinkBBW[:-4]), int(options.LinkCBW[:-4])])
    
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
    schedulerId_records = proprocess_schedulerId_data('/home/hong/workspace/mptcp/ns3/rl_training_data/' + options.EpisodeNum + '_schedulerId')
    AnalyzeMetaSocket(meta_socket_records, clientAvailableTxBuffer_records, schedulerId_records)
    sns.plt.savefig(os.path.join(options.DirPath, options.Experiment + "_" + options.Scheduler + '_meta_' + options.EpisodeNum + ".png"), dpi = 150, bbox_inches='tight')
    sns.plt.close()

    other_server_rx_record = extract_other_server_rx_record('/home/hong/workspace/mptcp/ns3/rl_training_data/' + options.EpisodeNum + '_mptcp_other_server_0')
    WriteOtherServerThroughputToCsv(other_server_rx_record, 0)
    other_server_rx_record = extract_other_server_rx_record('/home/hong/workspace/mptcp/ns3/rl_training_data/' + options.EpisodeNum + '_mptcp_other_server_1')
    WriteOtherServerThroughputToCsv(other_server_rx_record, 1)

    recordResultToCsv()
    print os.path.join(options.DirPath, options.Experiment + "_" + options.Scheduler + '_meta_' + options.EpisodeNum + ".png")
    # analyze_reward('/home/hong/workspace/mptcp/ns3/rl_training_data/' + options.EpisodeNum + '_calculate_reward')
