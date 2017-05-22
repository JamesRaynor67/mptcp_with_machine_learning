from operator import truediv
import numpy as np
# import seaborn as sns
import matplotlib.pyplot as plt

from log_helper_mptcp_subflow_id import MpTcpSubflows
from log_helper_smoother import Smooth
from log_helper import ConvertByteToBit

def AnalyzeMonitorSendingRate(record):
    time = [[],[],[],[]]
    tx, rx = [[],[],[],[]], [[],[],[],[]]
    tRate, rRate = [[],[],[],[]], [[],[],[],[]]
    smoothed_tRate, smoothed_rRate, smoothed_time = [[],[],[],[]], [[],[],[],[]], [[],[],[],[]]
    
    for row in record:
        # flow id is from 1 to N,
        if row[1] in MpTcpSubflows.getSubflowList():
            time[MpTcpSubflows.getSubflowId(row[1])].append(row[0]) # append time stamp for flow with id row[1]
            tx[MpTcpSubflows.getSubflowId(row[1])].append(row[3]) # append TxPacket num for flow with id row[1]
            rx[MpTcpSubflows.getSubflowId(row[1])].append(row[5]) # append TxPacket num for flow with id row[1]

    for i in range(4):
        tRate[i] = ConvertByteToBit(np.diff(tx[i])/np.diff(time[i]))
        rRate[i] = ConvertByteToBit(np.diff(rx[i])/np.diff(time[i]))
        smoothed_time[i], smoothed_rRate[i], smoothed_tRate[i] = Smooth(time[i], rRate[i], tRate[i])

    # c_s_subflow_1, = plt.plot(time[0][0:-1], tRate[0], 'ro', linewidth=1.0) # s->c 1
    # s_c_subflow_1, = plt.plot(time[1][0:-1], tRate[1], 'ro', linewidth=1.0) # c->s 1
    # c_s_subflow_2, = plt.plot(time[2][0:-1], tRate[2], 'bo', linewidth=1.0) # s->c 2
    # s_c_subflow_2, = plt.plot(time[3][0:-1], tRate[3], 'bo', linewidth=1.0) # c->s 2
    # plt.plot(smoothed_time[0], smoothed_tRate[0], 'r-', linewidth=2.0) # s->c 1
    # plt.plot(smoothed_time[1], smoothed_tRate[1], 'r-.', linewidth=2.0) # c->s 1
    # plt.plot(smoothed_time[2], smoothed_tRate[2], 'b-', linewidth=2.0) # s->c 2
    # plt.plot(smoothed_time[3], smoothed_tRate[3], 'b-.', linewidth=2.0) # c->s 2
    # plt.legend([c_s_subflow_1, s_c_subflow_1, c_s_subflow_2, s_c_subflow_2],
    #            ['server to client SendRate over subflow 1', 'client to server SendRate over subflow 1',
    #             'server to client SendRate over subflow 2', 'client to server SendRate over subflow 2'], loc='best')
    # plt.title('Time-TxRate')
    # plt.xlabel('Time / s', fontsize = 14, color = 'black')
    # plt.ylabel('SendRate', fontsize = 14, color = 'black')


    c_s_subflow_1, = plt.plot(time[0][0:-1], tRate[0], 'ro', linewidth=1.0) # s->c 1
    # s_c_subflow_1, = plt.plot(time[1][0:-1], tRate[1], 'ro', linewidth=1.0) # c->s 1
    c_s_subflow_2, = plt.plot(time[2][0:-1], tRate[2], 'bo', linewidth=1.0) # s->c 2
    # s_c_subflow_2, = plt.plot(time[3][0:-1], tRate[3], 'bo', linewidth=1.0) # c->s 2
    plt.plot(smoothed_time[0], smoothed_tRate[0], 'r-', linewidth=2.0) # s->c 1
    # plt.plot(smoothed_time[1], smoothed_tRate[1], 'r-.', linewidth=2.0) # c->s 1
    plt.plot(smoothed_time[2], smoothed_tRate[2], 'b-', linewidth=2.0) # s->c 2
    # plt.plot(smoothed_time[3], smoothed_tRate[3], 'b-.', linewidth=2.0) # c->s 2
    plt.legend([c_s_subflow_1, c_s_subflow_2],
               ['client to server SendRate over subflow 1', 'client to server SendRate over subflow 2'], loc='best')
    plt.title('Time-TxRate')
    plt.xlabel('Time / s', fontsize = 14, color = 'black')
    plt.ylabel('SendRate/ Bps', fontsize = 14, color = 'black')