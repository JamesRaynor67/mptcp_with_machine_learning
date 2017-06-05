import matplotlib.pyplot as plt
import seaborn as sns
from log_helper_mptcp_subflow_id import MpTcpSubflows

# Almost all factors influence RTT. And I think delay, bandwidth and application rate are most important

def AnalyzeClientRtt(rtt_records):
    client_rtt0 = rtt_records[["Timestamp", "Rtt0"]].values
    client_rtt1 = rtt_records[["Timestamp", "Rtt1"]].values

    c_s_subflow_1_rtt, = sns.plt.plot(list(client_rtt0[:,0]), list(client_rtt0[:,1]/1000), 'b-') # convert rtt time unit to millisecond
    c_s_subflow_2_rtt, = sns.plt.plot(list(client_rtt1[:,0]), list(client_rtt1[:,1]/1000), 'r-')

    sns.plt.legend([c_s_subflow_1_rtt, c_s_subflow_2_rtt], ['subflow 1 RTT', 'subflow 2 RTT'], loc='best')
    sns.plt.title('Time-RTT')
    sns.plt.xlabel('Time / s', fontsize = 14, color = 'black')
    sns.plt.ylabel('RTT / ms', fontsize = 14, color = 'black')

def AnalyzeClientCwnd(cWnd_records):
    client_cWnd0 = cWnd_records[["Timestamp", "cWnd0"]].values
    client_cWnd1 = cWnd_records[["Timestamp", "cWnd1"]].values

    c_s_subflow_1_cWnd, = sns.plt.plot(list(client_cWnd0[:,0]), list(client_cWnd0[:,1]), 'b-') # convert cWnd time unit to millisecond
    c_s_subflow_2_cWnd, = sns.plt.plot(list(client_cWnd1[:,0]), list(client_cWnd1[:,1]), 'r-')

    sns.plt.legend([c_s_subflow_1_cWnd, c_s_subflow_2_cWnd], ['subflow 1 cWnd', 'subflow 2 cWnd'], loc='best')
    sns.plt.title('Time-cWnd')
    sns.plt.xlabel('Time / s', fontsize = 14, color = 'black')
    sns.plt.ylabel('cWnd / byte', fontsize = 14, color = 'black')

def AnalyzeClientRwnd(rWnd_records):
    client_rWnd0 = rWnd_records[["Timestamp", "rWnd0"]].values
    client_rWnd1 = rWnd_records[["Timestamp", "rWnd1"]].values

    c_s_subflow_1_rWnd, = sns.plt.plot(list(client_rWnd0[:,0]), list(client_rWnd0[:,1]), 'b-') # convert cWnd time unit to millisecond
    c_s_subflow_2_rWnd, = sns.plt.plot(list(client_rWnd1[:,0]), list(client_rWnd1[:,1]), 'r-')

    sns.plt.legend([c_s_subflow_1_rWnd, c_s_subflow_2_rWnd], ['subflow 1 rWnd', 'subflow 2 rWnd'], loc='best')
    sns.plt.title('Time-rWnd')
    sns.plt.xlabel('Time / s', fontsize = 14, color = 'black')
    sns.plt.ylabel('rWnd / byte', fontsize = 14, color = 'black')

def AnalyzeClientUnAck(unAck_records):
    client_unAck0 = unAck_records[["Timestamp", "unAck0"]].values
    client_unAck1 = unAck_records[["Timestamp", "unAck1"]].values

    c_s_subflow_1_unAck, = sns.plt.plot(list(client_unAck0[:,0]), list(client_unAck0[:,1]), 'b-') # convert cWnd time unit to millisecond
    c_s_subflow_2_unAck, = sns.plt.plot(list(client_unAck1[:,0]), list(client_unAck1[:,1]), 'r-')

    sns.plt.legend([c_s_subflow_1_unAck, c_s_subflow_2_unAck], ['subflow 1 unAck', 'subflow 2 unAck'], loc='best')
    sns.plt.title('Time-unAck')
    sns.plt.xlabel('Time / s', fontsize = 14, color = 'black')
    sns.plt.ylabel('UnAck / byte', fontsize = 14, color = 'black')

def AnalyzeMetaSocket(meta_socket_records):
    # columns = ['Timestamp', 'LastAckedSeq', 'HighTxMark', 'AvailableTxBuffer', 'NextTxSeq', 'TotalCwnd']
    data = meta_socket_records.values
    
    sns.plt.figure(figsize=(16*2, 9*2))

    sns.plt.subplot(3,2,1)
    meta_lastAck, = sns.plt.plot(list(data[:,0]), list(data[:,1]), 'k-')
    sns.plt.legend([meta_lastAck], ['Meta-Socket Last Acked Sequence'], loc='best')
    sns.plt.title('Time-lastAck')
    sns.plt.xlabel('Time / s', fontsize = 14, color = 'black')
    sns.plt.ylabel('Last Ack Sequence Number / byte', fontsize = 14, color = 'black')

    sns.plt.subplot(3,2,2)
    meta_highTxMark, = sns.plt.plot(list(data[:,0]), list(data[:,2]), 'k-')
    sns.plt.legend([meta_highTxMark], ['Meta-Socket Highest Tx Sequence Number'], loc='best')
    sns.plt.title('Time-highTxMark')
    sns.plt.xlabel('Time / s', fontsize = 14, color = 'black')
    sns.plt.ylabel('Highest Tx Sequence Number / byte', fontsize = 14, color = 'black')

    sns.plt.subplot(3,2,3)
    meta_availableTxBuffer, = sns.plt.plot(list(data[:,0]), list(data[:,3]), 'k-')
    sns.plt.legend([meta_availableTxBuffer], ['Meta-Socket Available TxBuffer Size'], loc='best')
    sns.plt.title('Time-availableTxBuffer')
    sns.plt.xlabel('Time / s', fontsize = 14, color = 'black')
    sns.plt.ylabel('Available TxBuffer Size / byte', fontsize = 14, color = 'black')

    sns.plt.subplot(3,2,4)
    meta_nextTxSeq, = sns.plt.plot(list(data[:,0]), list(data[:,4]), 'k-')
    sns.plt.legend([meta_nextTxSeq], ['Meta-Socket Next Transmission Sequence'], loc='best')
    sns.plt.title('Time-nextTxSeq')
    sns.plt.xlabel('Time / s', fontsize = 14, color = 'black')
    sns.plt.ylabel('Next Tx Sequence Number / byte', fontsize = 14, color = 'black')

    sns.plt.subplot(3,2,5)
    meta_totalCwnd, = sns.plt.plot(list(data[:,0]), list(data[:,5]), 'k-')
    sns.plt.legend([meta_totalCwnd], ['Meta-Socket Total cWnd Size'], loc='best')
    sns.plt.title('Time-totalCwnd')
    sns.plt.xlabel('Time / s', fontsize = 14, color = 'black')
    sns.plt.ylabel('Total cWnd Size / byte', fontsize = 14, color = 'black')

    sns.plt.subplot(3,2,6)
    meta_unAck, = sns.plt.plot(list(data[:,0]), list(data[:,6]), 'k-')
    sns.plt.legend([meta_unAck], ['Meta-Socket unAck'], loc='best')
    sns.plt.title('Time-UnAck')
    sns.plt.xlabel('Time / s', fontsize = 14, color = 'black')
    sns.plt.ylabel('UnAck Size / byte', fontsize = 14, color = 'black')



# def AnalyzeClientRtt(rtt_records):
#     client_rtt0 = rtt_records[["Timestamp", "Rtt0"]].values
#     client_rtt1 = rtt_records[["Timestamp", "Rtt1"]].values

#     c_s_subflow_1_rtt, = sns.plt.plot(list(client_rtt0[:,0]), list(client_rtt0[:,1]/1000), 'b-') # convert rtt time unit to millisecond
#     c_s_subflow_2_rtt, = sns.plt.plot(list(client_rtt1[:,0]), list(client_rtt1[:,1]/1000), 'r-')

#     sns.plt.legend([c_s_subflow_1_rtt, c_s_subflow_2_rtt], ['subflow 1 RTT', 'subflow 2 RTT'], loc='best')
#     sns.plt.title('Time-RTT')
#     sns.plt.xlabel('Time / s', fontsize = 14, color = 'black')
#     sns.plt.ylabel('RTT / ms', fontsize = 14, color = 'black')