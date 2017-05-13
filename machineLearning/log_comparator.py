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
                timestamp = int(row[0])/10e8
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
    mptcp_subflow_id = [-1]*4
    record = []
    with open(file_path, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',')
        next(spamreader)
        for row in spamreader:
            timestamp = int(row[0])/10e8    
            flowId = int(row[1])
            From = row[2]
            To = row[3]
            TxPacket = int(row[4])
            RxPacket = int(row[6])
            delaySum = float(row[8][1:-2])/10e8
            lostPackets = int(row[10])

            record.append([timestamp, flowId, TxPacket, RxPacket, delaySum, lostPackets])
            if -1 in mptcp_subflow_id:
                if flowId not in mptcp_subflow_id:
                    if From == '192.168.0.1' and To == '192.168.9.2':
                        mptcp_subflow_id[1] = flowId
                    elif From == '192.168.9.2' and To == '192.168.0.1':
                        mptcp_subflow_id[0] = flowId
                    elif From == '192.168.0.1' and To == '192.168.11.2':
                        mptcp_subflow_id[3] = flowId
                    elif From == '192.168.11.2' and To == '192.168.0.1':
                        mptcp_subflow_id[2] = flowId

    print 'mptcp subflow ids: ', mptcp_subflow_id
    record.sort(key=lambda ele:ele[0])
    
    x = [[],[],[],[]]
    y = [[],[],[],[]]
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
    x1, x2, x3, x4 = flow_data[0][0][0], flow_data[0][0][1], flow_data[0][0][2], flow_data[0][0][3]
    y1, y2, y3, y4 = flow_data[0][1][0], flow_data[0][1][1], flow_data[0][1][2], flow_data[0][1][3]
    x, y = combine_two_subflow_records(x1, y1, x3, y3)
    rr, = plt.plot(x, y, 'g-', label = 'Round Robin total: ' + str(y[-1]), linewidth=3.0)
    rr1, = plt.plot(x1, y1, 'g-.', label = 'Round Robin subflow 1: ' + str(y1[-1]), linewidth=2.0)
    rr3, = plt.plot(x3, y3, 'g:', label = 'Round Robin subflow 2: ' + str(y3[-1]), linewidth=2.0)

    
    x1, x2, x3, x4 = flow_data[1][0][0], flow_data[1][0][1], flow_data[1][0][2], flow_data[1][0][3]
    y1, y2, y3, y4 = flow_data[1][1][0], flow_data[1][1][1], flow_data[1][1][2], flow_data[1][1][3]
    x, y = combine_two_subflow_records(x1, y1, x3, y3)
    rtt, = plt.plot(x, y, 'r-', label = 'Fastest RTT total: ' + str(y[-1]), linewidth=3.0)
    rtt1, = plt.plot(x1, y1, 'r-.', label = 'Fastest RTT subflow 1: ' + str(y1[-1]), linewidth=2.0)
    rtt3, = plt.plot(x3, y3, 'r:', label = 'Fastest RTT subflow 2: ' + str(y3[-1]), linewidth=2.0)

    x1, x2, x3, x4 = flow_data[2][0][0], flow_data[2][0][1], flow_data[2][0][2], flow_data[2][0][3]
    y1, y2, y3, y4 = flow_data[2][1][0], flow_data[2][1][1], flow_data[2][1][2], flow_data[2][1][3]
    x, y = combine_two_subflow_records(x1, y1, x3, y3)
    rd, = plt.plot(x, y, 'b-', label = 'Random total: ' + str(y[-1]), linewidth=3.0)    
    rd1, = plt.plot(x1, y1, 'b-.', label = 'Random subflow 1: ' + str(y1[-1]), linewidth=2.0)
    rd3, = plt.plot(x3, y3, 'b:', label = 'Random subflow 2: ' + str(y3[-1]), linewidth=2.0)

    x1, x2, x3, x4 = flow_data[3][0][0], flow_data[3][0][1], flow_data[3][0][2], flow_data[3][0][3]
    y1, y2, y3, y4 = flow_data[3][1][0], flow_data[3][1][1], flow_data[3][1][2], flow_data[3][1][3]
    x, y = combine_two_subflow_records(x1, y1, x3, y3)
    ldbp, = plt.plot(x, y, 'y-', label = 'Largest DBP total: ' + str(y[-1]), linewidth=3.0)    
    ldbp1, = plt.plot(x1, y1, 'y-.', label = 'Largest DBP subflow 1: ' + str(y1[-1]), linewidth=2.0)
    ldbp3, = plt.plot(x3, y3, 'y:', label = 'Largest DBP subflow 2: ' + str(y3[-1]), linewidth=2.0)

    plt.legend([rr, rr1, rr3, rtt, rtt1, rtt3, rd, rd1, rd3, ldbp, ldbp1, ldbp3], loc='upper left')

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
