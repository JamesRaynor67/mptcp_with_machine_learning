import csv
import matplotlib.pyplot as plt
import sys
import itertools

def analyze_application(file_path):
    record = []
    with open(file_path, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',')
        next(spamreader)
        total_psize = 0
        for row in spamreader:
            if int(row[1]) == 1: # not send record
                timestamp = int(row[0])/1e9
                total_psize += int(row[7])
                record.append([timestamp, total_psize])

    record.sort(key=lambda ele:ele[0])
    x, y = [], []
    for pair in record:
        x.append(pair[0])
        y.append(pair[1])
    return [x, y]
    # sent_packet_size, = plt.plot(x, y, 'go')
    # plt.legend([sent_packet_size], ['sent packet size'], loc='upper left')
    # plt.title('Time-Sent packet size')
    # plt.xlabel('Time / s', fontsize = 14, color = 'black')
    # plt.ylabel('Sent packet size / Byte', fontsize = 14, color = 'black')
    # print 'server send total: ', y[-1], ' Bytes'

def analyze_flow(file_path):
    mptcp_subflow_id = [-1]*8
    record = []
    with open(file_path, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',')
        next(spamreader)
        for row in spamreader:
            timestamp = int(row[0])/1e9    
            flowId = int(row[1])
            From = row[2]
            To = row[3]
            TxPacket = int(row[4])
            RxPacket = int(row[6])
            delaySum = float(row[8][1:-2])/1e9
            lostPackets = int(row[10])

            record.append([timestamp, flowId, TxPacket, RxPacket, delaySum, lostPackets])
            if -1 in mptcp_subflow_id:
                if flowId not in mptcp_subflow_id:
                    if From == '192.168.0.1' and To == '192.168.9.2':
                        mptcp_subflow_id[1] = flowId
                    elif From == '192.168.9.2' and To == '192.168.0.1': # !
                        mptcp_subflow_id[0] = flowId
                    elif From == '192.168.0.1' and To == '192.168.11.2':
                        mptcp_subflow_id[3] = flowId
                    elif From == '192.168.11.2' and To == '192.168.0.1': # !
                        mptcp_subflow_id[2] = flowId
                    elif From == '192.168.0.2' and To == '192.168.1.2': # !
                        mptcp_subflow_id[4] = flowId
                    elif From == '192.168.1.2' and To == '192.168.0.2':
                        mptcp_subflow_id[5] = flowId
                    elif From == '192.168.5.2' and To == '192.168.4.2': # !
                        mptcp_subflow_id[6] = flowId
                    elif From == '192.168.4.2' and To == '192.168.5.2':
                        mptcp_subflow_id[7] = flowId                        

    print 'mptcp subflow ids: ', mptcp_subflow_id
    assert ((-1 in mptcp_subflow_id) is False)
    record.sort(key=lambda ele:ele[0])
    
    x ,y = [[] for i in range(8)], [[] for i in range(8)]
    for row in record:
        # flow id is from 1 to N,
        if row[1] in mptcp_subflow_id:
            x[mptcp_subflow_id.index(row[1])].append(row[0]) # append time stamp for flow with id row[1]
            y[mptcp_subflow_id.index(row[1])].append(row[2]) # append TxPacket num for flow with id row[1]

    return [x, y]
    # s_c_subflow_1, = plt.plot(x[0], y[0], 'r-', linewidth=2.0) # s->c 1
    # c_s_subflow_1, = plt.plot(x[1], y[1], 'r-.', linewidth=2.0) # c->s 1
    # s_c_subflow_2, = plt.plot(x[2], y[2], 'b-', linewidth=2.0) # s->c 2
    # c_s_subflow_2, = plt.plot(x[3], y[3], 'b-.', linewidth=2.0) # c->s 2
    # plt.legend([s_c_subflow_1, c_s_subflow_1, s_c_subflow_2, c_s_subflow_2],
    #            ['server to client packet number over subflow 1', 'client to server packet number over subflow 1',
    #             'server to client packet number over subflow 2', 'client to server packet number over subflow 2'], loc='upper left')
    # plt.title('Time-TxPacket')
    # plt.xlabel('Time / s', fontsize = 14, color = 'black')
    # plt.ylabel('Packet number', fontsize = 14, color = 'black')

def plot_application(application_data):
    x, y = application_data[0][0], application_data[0][1]
    rr, = plt.plot(x, y, 'g-', label = 'Round Robin sent packet size: ' + str(y[-1]) + ' Bytes', linewidth=2.0)
    

    x, y = application_data[1][0], application_data[1][1]
    rtt, = plt.plot(x, y, 'r--', label = 'Fastest RTT sent packet size: ' + str(y[-1]) + ' Bytes', linewidth=2.0)

    x, y = application_data[2][0], application_data[2][1]
    rd, = plt.plot(x, y, 'b-.', label = 'Random sent packet size: ' + str(y[-1]) + ' Bytes', linewidth=2.0)

    x, y = application_data[3][0], application_data[3][1]
    ldbp, = plt.plot(x, y, 'y:', label = 'Largest DBP sent packet size: ' + str(y[-1]) + ' Bytes', linewidth=2.0)
    plt.legend([rr, rtt, rd, ldbp], loc='upper left')

    plt.title('Time-Sent packet size')
    plt.xlabel('Time / s', fontsize = 14, color = 'black')
    plt.ylabel('Sent packet size / Byte', fontsize = 14, color = 'black')

def combine_two_subflow_records(x1, y1, x2, y2):
    x, y = [], []
    index_1, index_2 = 0, 0
    while index_1 < len(x1) or index_2 < len(x2):
        if index_1 == len(x1):
            x.append(x2[index_2]); y.append(y1[index_1] + y2[index_2]);
            index_2 += 1
        elif index_2 == len(x2):
            x.append(x1[index_1]); y.append(y1[index_1] + y2[index_2]);
            index_1 += 1
        else:
            if x1[index_1] < x2[index_2]:
                x.append(x1[index_1]); y.append(y1[index_1] + y2[index_2]);
                index_1 += 1
            elif x1[index_1] > x2[index_2]:
                x.append(x2[index_2]); y.append(y1[index_1] + y2[index_2]);
                index_2 += 1
            else: # x1[index_1] == x2[index_2]:
                x.append(x1[index_1]); y.append(y1[index_1] + y2[index_2]);
                index_1 += 1; index_2 += 1;
    return x, y

def plot_flow(flow_data):
    x0, x2, x4, x6 = flow_data[0][0][0], flow_data[0][0][2], flow_data[0][0][4], flow_data[0][0][6]
    y0, y2, y4, y6 = flow_data[0][1][0], flow_data[0][1][2], flow_data[0][1][4], flow_data[0][1][6]
    x, y = combine_two_subflow_records(x0, y0, x2, y2)
    x_others, y_others = combine_two_subflow_records(x4, y4, x6, y6)
    rr, = plt.plot(x, y, 'g-', label = 'Round Robin total: ' + str(y[-1]), linewidth=3.0)
    rr_others, = plt.plot(x_others, y_others, 'g.', label = 'Round Robin others total: ' + str(y_others[-1]), linewidth=2.0)
    # rr0, = plt.plot(x0, y0, 'g-.', label = 'Round Robin subflow 1: ' + str(y0[-1]), linewidth=2.0)
    # rr2, = plt.plot(x2, y2, 'g:', label = 'Round Robin subflow 2: ' + str(y2[-1]), linewidth=2.0)
    rr4, = plt.plot(x4, y4, 'g-.', label = 'Round Robin other traffic 1: ' + str(y4[-1]), linewidth=2.0)
    rr6, = plt.plot(x6, y6, 'g:', label = 'Round Robin other traffic 2: ' + str(y6[-1]), linewidth=2.0)
    
    x0, x2, x4, x6 = flow_data[1][0][0], flow_data[1][0][2], flow_data[1][0][4], flow_data[1][0][6]
    y0, y2, y4, y6 = flow_data[1][1][0], flow_data[1][1][2], flow_data[1][1][4], flow_data[1][1][6]
    x, y = combine_two_subflow_records(x0, y0, x2, y2)
    x_others, y_others = combine_two_subflow_records(x4, y4, x6, y6)
    rtt, = plt.plot(x, y, 'r-', label = 'Fastest RTT total: ' + str(y[-1]), linewidth=3.0)
    rtt_others, = plt.plot(x_others, y_others, 'r.', label = 'Fastest RTT others total: ' + str(y_others[-1]), linewidth=2.0)
    # rtt0, = plt.plot(x0, y0, 'r-.', label = 'Fastest RTT subflow 1: ' + str(y0[-1]), linewidth=2.0)
    # rtt2, = plt.plot(x2, y2, 'r:', label = 'Fastest RTT subflow 2: ' + str(y2[-1]), linewidth=2.0)
    rtt4, = plt.plot(x4, y4, 'r-.', label = 'Round Robin other traffic 1: ' + str(y4[-1]), linewidth=2.0)
    rtt6, = plt.plot(x6, y6, 'r:', label = 'Round Robin other traffic 2: ' + str(y6[-1]), linewidth=2.0)

    x0, x2, x4, x6 = flow_data[2][0][0], flow_data[2][0][2], flow_data[2][0][4], flow_data[2][0][6]
    y0, y2, y4, y6 = flow_data[2][1][0], flow_data[2][1][2], flow_data[2][1][4], flow_data[2][1][6]
    x, y = combine_two_subflow_records(x0, y0, x2, y2)
    x_others, y_others = combine_two_subflow_records(x4, y4, x6, y6)
    rd, = plt.plot(x, y, 'b-', label = 'Random total: ' + str(y[-1]), linewidth=3.0)    
    rd_others, = plt.plot(x_others, y_others, 'b.', label = 'Random others total: ' + str(y_others[-1]), linewidth=2.0)    
    # rd0, = plt.plot(x0, y0, 'b-.', label = 'Random subflow 1: ' + str(y0[-1]), linewidth=2.0)
    # rd2, = plt.plot(x2, y2, 'b:', label = 'Random subflow 2: ' + str(y2[-1]), linewidth=2.0)
    rd4, = plt.plot(x4, y4, 'b-.', label = 'Round Robin other traffic 1: ' + str(y4[-1]), linewidth=2.0)
    rd6, = plt.plot(x6, y6, 'b:', label = 'Round Robin other traffic 2: ' + str(y6[-1]), linewidth=2.0)

    x0, x2, x4, x6 = flow_data[3][0][0], flow_data[3][0][2], flow_data[3][0][4], flow_data[3][0][6]
    y0, y2, y4, y6 = flow_data[3][1][0], flow_data[3][1][2], flow_data[3][1][4], flow_data[3][1][6]
    x, y = combine_two_subflow_records(x0, y0, x2, y2)
    x_others, y_others = combine_two_subflow_records(x4, y4, x6, y6)
    ldbp, = plt.plot(x, y, 'y-', label = 'Largest DBP total: ' + str(y[-1]), linewidth=3.0)    
    ldbp_others, = plt.plot(x_others, y_others, 'y.', label = 'Largest DBP others total: ' + str(y_others[-1]), linewidth=2.0)    
    # ldbp0, = plt.plot(x0, y0, 'y-.', label = 'Largest DBP subflow 1: ' + str(y0[-1]), linewidth=2.0)
    # ldbp3, = plt.plot(x2, y2, 'y:', label = 'Largest DBP subflow 2: ' + str(y2[-1]), linewidth=2.0)
    ldbp4, = plt.plot(x4, y4, 'y-.', label = 'Round Robin other traffic 1: ' + str(y4[-1]), linewidth=2.0)
    ldbp6, = plt.plot(x6, y6, 'y:', label = 'Round Robin other traffic 2: ' + str(y6[-1]), linewidth=2.0)

    # plt.legend([rr, rr0, rr2, rtt, rtt0, rtt2, rd, rd0, rd2, ldbp, ldbp0, ldbp2], loc='upper left')
    plt.legend([rr, rr_others, rr4, rr6, rtt, rtt_others, rtt4, rtt6, rd, rd_others, rd4, rd6, ldbp, ldbp_others, ldbp4, ldbp6], loc='upper left')

    plt.title('Time-Sent packet number')
    plt.xlabel('Time / s', fontsize = 14, color = 'black')
    plt.ylabel('Sent packet number', fontsize = 14, color = 'black')

if __name__ == '__main__':

    catagolary_num = int(sys.argv[1]) # usually is 4
    application_data = []
    server_to_client_data = []
    for i in range(catagolary_num):
        file_path = sys.argv[i + 2] + '/mptcp_server'
        application_data.append(analyze_application(file_path))

        file_path = sys.argv[i + 2] + '/mptcp_server_cWnd'
        server_to_client_data.append(analyze_flow(file_path))

    plt.subplot(2,1,1)
    plot_application(application_data)
    plt.subplot(2,1,2)
    plot_flow(server_to_client_data)
    # analyze_application('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(batch_num) + '_mptcp_server')
    # # analyze_application('/home/hong/workspace/mptcp/ns3/rl_training_data_wrong/' + str(batch_num) + '_mptcp_server')
    # # analyze_flow('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(batch_num) + '_mptcp_server_cWnd')
    # # analyze_reward('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(batch_num) + '_calculate_reward')
    # plt.subplot(4,1,2)
    # analyze_client_end_node('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(batch_num) + '_mptcp_client')
    # plt.subplot(4,1,3)
    # analyze_server_end_point('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(batch_num) + '_mptcp_server')
    # plt.subplot(4,1,4)
    # analyze_flow('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(batch_num) + '_mptcp_server_cWnd')
    plt.show()
