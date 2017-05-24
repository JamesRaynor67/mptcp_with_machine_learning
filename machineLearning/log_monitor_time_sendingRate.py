from operator import truediv
import numpy as np
import seaborn as sns
import matplotlib.pyplot as plt

from log_helper_mptcp_subflow_id import MpTcpSubflows
from log_helper_smoother import SmoothRate
from log_helper import ConvertByteToBit
from log_helper import GetRate

def AnalyzeMonitorSendingRate(monitor_records):
    subflows = [None] * 4
    smoothed_subflows = [None] * 4
    
    for index in range(4):
        subflows[index] = monitor_records.loc[monitor_records["SubflowId"] == index, ["Timestamp", "TxBytes", "RxBytes", "SubflowId"]]

    subflow_rates = GetRate(subflows)
    smoothed_subflow_rates = SmoothRate(subflow_rates, groupSize = 10, sampleInteval = 1)

    # devide 1000 to turn unit into Kbps
    c_s_subflow_1, = sns.plt.plot(subflow_rates[0]["Timestamp"].values, subflow_rates[0]["TxRate"].values/1000.0,'ro', linewidth=.2) # s->c 1
    # s_c_subflow_1, = plt.plot(time[1][0:-1], tRate[1], 'ro', linewidth=1.0) # c->s 1
    c_s_subflow_2, = sns.plt.plot(subflow_rates[2]["Timestamp"].values, subflow_rates[2]["TxRate"].values/1000.0, 'bo', linewidth=.2) # s->c 2
    # s_c_subflow_2, = plt.plot(time[3][0:-1], tRate[3], 'bo', linewidth=1.0) # c->s 2
    sns.plt.plot(smoothed_subflow_rates[0]["Timestamp"].values, smoothed_subflow_rates[0]["TxRate"].values/1000.0, 'r-', linewidth=2.0) # s->c 1
    # plt.plot(smoothed_time[1], smoothed_tRate[1], 'r-.', linewidth=2.0) # c->s 1
    sns.plt.plot(smoothed_subflow_rates[2]["Timestamp"].values, smoothed_subflow_rates[2]["TxRate"].values/1000.0, 'b-', linewidth=2.0) # s->c 2
    # plt.plot(smoothed_time[3], smoothed_tRate[3], 'b-.', linewidth=2.0) # c->s 2
    sns.plt.legend([c_s_subflow_1, c_s_subflow_2], ['client to server SendRate over subflow 1', 'client to server SendRate over subflow 2'], loc='best')
    sns.plt.title('Time-TxRate')
    sns.plt.xlabel('Time / s', fontsize = 14, color = 'black')
    sns.plt.ylabel('SendRate/ Kbps', fontsize = 14, color = 'black')
    return subflow_rates, smoothed_subflow_rates

def AnalyzeMonitorSendingRateUtilization(subflow_rates, smoothed_subflow_rates, linkBWList):
    smoothed_subflow_rates = SmoothRate(subflow_rates, groupSize = 30, sampleInteval = 3)
    c_s_subflow_1, = sns.plt.plot(smoothed_subflow_rates[0]["Timestamp"].values, smoothed_subflow_rates[0]["TxRate"].values/1000.0/linkBWList[0], 'r-', linewidth=2.0) # s->c 1
    c_s_subflow_2, = sns.plt.plot(smoothed_subflow_rates[2]["Timestamp"].values, smoothed_subflow_rates[2]["TxRate"].values/1000.0/linkBWList[1], 'b-', linewidth=2.0) # s->c 2

    sns.plt.legend([c_s_subflow_1, c_s_subflow_2], ['subflow 1 sending rate/link a bandwidth', 'subflow 2 sending rate/link a bandwidth'], loc='best')
    sns.plt.title('Time-Utilization')
    sns.plt.xlabel('Time / s', fontsize = 14, color = 'black')
    sns.plt.ylabel('Percentage', fontsize = 14, color = 'black')

