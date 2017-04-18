import socket
import pandas
from time import sleep
from rl_socket import Interacter_socket
from RL_core import QLearningTable
from RL_core import extract_observation
from RL_core import action_translator
from RL_core import calculate_reward
from RL_core import apply_action
from shutil import copyfile

def IsInt(s):
    # A naive method, but enough here
    if "." in s:
        return False
    else:
        return True

class DataRecorder():

    def __init__(self):
        self.next_seq_num = 0
        self.subflow_data = {}
        self.action = []

    def add_one_record(self, str_data):
        # name$value#name$value...
        pair_list = str_data.split("$")
        one_row_of_train_data = {}
        for pair in pair_list:
            if len(pair) > 3: # this ensures the string (pair) is not empty or string with single '$'
                name_val_list = pair.split("#")
                # print "Hong Jiaming: 1 ", pair, len(name_val_list), name_val_list[0], name_val_list[1]
                if IsInt(name_val_list[1]):
                    one_row_of_train_data[name_val_list[0]] = int(name_val_list[1])
                else:
                    one_row_of_train_data[name_val_list[0]] = float(name_val_list[1])

        # ensure this transmission is right and complete
        # neighbour TCP segments must not combined into one
        assert one_row_of_train_data["size"] == len(one_row_of_train_data)
        assert one_row_of_train_data["ssn"] == self.next_seq_num
        self.subflow_data[self.next_seq_num] = one_row_of_train_data
        self.next_seq_num += 1

    def get_subflow_data_dic(self):
        return self.subflow_data

    def get_latest_subflow_data(self):
        return self.subflow_data[self.next_seq_num-1]

    def add_pair_to_last_record(self, name, value):
        self.subflow_data[self.next_seq_num-1][name] = value

    def print_all_subflow_data(self):
        print "dic size: ", len(self.subflow_data)
        for ssn, data in self.subflow_data.iteritems():
            for k, v in data.iteritems():
                print "key: ", k, "value: ", v

    def print_latest_subflow_data(self):
        latest_data = self.subflow_data[self.next_seq_num-1]
        for k, v in latest_data.iteritems():
            print "key: ", k, "value: ", v


if __name__ == "__main__":
    episode_count = 0
    RL = QLearningTable(actions=["use subflow 0", "use subflow 1"])

    while episode_count < 1000:
        interacter_socket = Interacter_socket(host = '', port = 12345)
        dataRecorder = DataRecorder()
        interacter_socket.listen()
        recv_str, this_batch_done = interacter_socket.recv()
        dataRecorder.add_one_record(recv_str)
        print 'iter: ', episode_count
        f = open("/home/hong/workspace/mptcp/ns3/mptcp_output/calculate_reward", 'w')
        f.write("time,reward\n")
        while True:
            observation_before_action = extract_observation(dataRecorder)
            # print observation_before_action
            action = RL.choose_action(observation_before_action)

            # print action
            apply_action(interacter_socket, dataRecorder, action)
            recv_str, this_batch_done = interacter_socket.recv() # get new observation and reward

            if this_batch_done is True:
                break

            dataRecorder.add_one_record(recv_str)
            observation_after_action = extract_observation(dataRecorder)

            reward = calculate_reward(dataRecorder)
            f.write(str(dataRecorder.get_latest_subflow_data()["time"]) + ',' + str(reward) + '\n')
            RL.learn(observation_before_action, action, reward, observation_after_action) # RL learning

            observation_before_action = observation_after_action
        interacter_socket.close()
        interacter_socket = None
        f.close()
        RL.q_table.to_csv("/home/hong/workspace/mptcp/ns3/mptcp_output/q_table")
        copyfile("/home/hong/workspace/mptcp/ns3/mptcp_output/calculate_reward", '/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_calculate_reward')
        copyfile("/home/hong/workspace/mptcp/ns3/mptcp_output/mptcp_client", '/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_mptcp_client')
        copyfile("/home/hong/workspace/mptcp/ns3/mptcp_output/mptcp_drops", '/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_mptcp_drops')
        copyfile("/home/hong/workspace/mptcp/ns3/mptcp_output/mptcp_server", '/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_mptcp_server')
        copyfile("/home/hong/workspace/mptcp/ns3/mptcp_output/mptcp_server_cWnd", '/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_mptcp_server_cWnd')
        # print "sleep 30 seconds from now"
        # sleep(30)
        episode_count += 1
