import matplotlib.pyplot as plt
import seaborn as sns
from log_helper_mptcp_subflow_id import MpTcpSubflows

def AnalyzeMonitorSentBytes(monitor_records):
    # ax = sns.tsplot(time="Timestamp", value="TxBytes", condition="SubflowId", data=monitor_records[monitor_records["SubflowId"] >= 0])
    # sns.factorplot(x="Timestamp", y="TxBytes", hue="SubflowId", data=monitor_records[monitor_records["SubflowId"] >= 0])
    # print monitor_records.loc[monitor_records["SubflowId"] == 0, ["Timestamp", "TxBytes"]].values[:,0], monitor_records.loc[monitor_records["SubflowId"] == 0, ["Timestamp", "TxBytes"]].values[:,]
    subflow0_sent = monitor_records.loc[monitor_records["SubflowId"] == 0, ["Timestamp", "TxBytes"]].values
    # subflow1 = monitor_records.loc[monitor_records["SubflowId"] == 1, ["Timestamp", "TxBytes"]].values
    subflow2_sent = monitor_records.loc[monitor_records["SubflowId"] == 2, ["Timestamp", "TxBytes"]].values
    # subflow3 = monitor_records.loc[monitor_records["SubflowId"] == 3, ["Timestamp", "TxBytes"]].values

    c_s_subflow_1, = sns.plt.plot(subflow0_sent[:,0], subflow0_sent[:,1])
    # sns.plt.plot(subflow1[:,0], subflow1[:,1])
    c_s_subflow_2, = sns.plt.plot(subflow2_sent[:,0], subflow2_sent[:,1])
    # sns.plt.plot(subflow3[:,0], subflow3[:,1])
    sns.plt.legend([c_s_subflow_1, c_s_subflow_2], ['subflow 1 sent bytes', 'subflow 2 sent bytes'], loc='best')
    sns.plt.title('Time-Sent Data Size')
    sns.plt.xlabel('Time / s', fontsize = 14, color = 'black')
    sns.plt.ylabel('Sent Data Size / Bytes', fontsize = 14, color = 'black')
    # x = [[],[],[],[]]
    # y = [[],[],[],[]]
    # for row in record:
    #     # flow id is from 1 to N,
    #     if row[1] in MpTcpSubflows.getSubflowList():
    #         x[MpTcpSubflows.getSubflowId(row[1])].append(row[0]) # append time stamp for flow with id row[1]
    #         y[MpTcpSubflows.getSubflowId(row[1])].append(row[2]) # append TxPacket num for flow with id row[1]

    # c_s_subflow_1, = plt.plot(x[0], y[0], 'r-', linewidth=2.0) # c->s 1
    # s_c_subflow_1, = plt.plot(x[1], y[1], 'r-.', linewidth=2.0) # s->c 1
    # c_s_subflow_2, = plt.plot(x[2], y[2], 'b-', linewidth=2.0) # c->s 2
    # s_c_subflow_2, = plt.plot(x[3], y[3], 'b-.', linewidth=2.0) # s->c 2
    # plt.legend([c_s_subflow_1, s_c_subflow_1, c_s_subflow_2, s_c_subflow_2],
    #            ['client to server packet number over subflow 1', 'server to client packet number over subflow 1',
    #             'client to server packet number over subflow 2', 'server to client packet number over subflow 2'], loc='best')
    # plt.title('Time-TxPacket')
    # plt.xlabel('Time / s', fontsize = 14, color = 'black')
    # plt.ylabel('Packet number', fontsize = 14, color = 'black')