import matplotlib.pyplot as plt
from log_helper_mptcp_subflow_id import MpTcpSubflows

def AnalyzeMonitorSentBytes(record):
    x = [[],[],[],[]]
    y = [[],[],[],[]]
    for row in record:
        # flow id is from 1 to N,
        if row[1] in MpTcpSubflows.getSubflowList():
            x[MpTcpSubflows.getSubflowId(row[1])].append(row[0]) # append time stamp for flow with id row[1]
            y[MpTcpSubflows.getSubflowId(row[1])].append(row[2]) # append TxPacket num for flow with id row[1]

    c_s_subflow_1, = plt.plot(x[0], y[0], 'r-', linewidth=2.0) # c->s 1
    s_c_subflow_1, = plt.plot(x[1], y[1], 'r-.', linewidth=2.0) # s->c 1
    c_s_subflow_2, = plt.plot(x[2], y[2], 'b-', linewidth=2.0) # c->s 2
    s_c_subflow_2, = plt.plot(x[3], y[3], 'b-.', linewidth=2.0) # s->c 2
    plt.legend([c_s_subflow_1, s_c_subflow_1, c_s_subflow_2, s_c_subflow_2],
               ['client to server packet number over subflow 1', 'server to client packet number over subflow 1',
                'client to server packet number over subflow 2', 'server to client packet number over subflow 2'], loc='best')
    plt.title('Time-TxPacket')
    plt.xlabel('Time / s', fontsize = 14, color = 'black')
    plt.ylabel('Packet number', fontsize = 14, color = 'black')