import matplotlib.pyplot as plt
import seaborn as sns
from log_helper_mptcp_subflow_id import MpTcpSubflows

# Almost all factors influence RTT. And I think delay, bandwidth and application rate are most important

def AnalyzeClientRtt(rtt_records):
    client_rtt0 = rtt_records[["Timestamp", "Rtt0"]].values
    client_rtt1 = rtt_records[["Timestamp", "Rtt1"]].values

    c_s_subflow_1_rtt, = sns.plt.plot(client_rtt0[:,0], client_rtt0[:,1]/1000) # convert rtt time unit to millisecond
    c_s_subflow_2_rtt, = sns.plt.plot(client_rtt1[:,0], client_rtt1[:,1]/1000)

    sns.plt.legend([c_s_subflow_1_rtt, c_s_subflow_2_rtt], ['subflow 1 RTT', 'subflow 2 RTT'], loc='best')
    sns.plt.title('Time-RTT')
    sns.plt.xlabel('Time / s', fontsize = 14, color = 'black')
    sns.plt.ylabel('RTT / ms', fontsize = 14, color = 'black')
