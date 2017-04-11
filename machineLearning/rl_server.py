import socket
from time import sleep
from RL_core import QLearningTable
from RL_core import extract_observation
from RL_core import action_translator
from RL_core import calculate_reward

def IsInt(s):
    # A naive method, but enough here
    if "." in s:
        return False
    else:
        return True

class Train_data():

    def __init__(self):
        self.next_seq_num = 0
        self.train_data = {}

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
        # if read too fast, consider force sleep before read
        assert one_row_of_train_data["size"] == len(one_row_of_train_data)
        assert one_row_of_train_data["ssn"] == self.next_seq_num
        self.train_data[self.next_seq_num] = one_row_of_train_data
        self.next_seq_num += 1

    def get_train_data_dic(self):
        return self.train_data

    def get_latest_train_data(self):
        return self.train_data[self.next_seq_num-1]

    def print_all_train_data(self):
        print "dic size: ", len(self.train_data)
        for ssn, data in self.train_data.iteritems():
            for k, v in data.iteritems():
                print "key: ", k, "value: ", v

    def print_latest_train_data(self):
        latest_data = self.train_data[self.next_seq_num-1]
        for k, v in latest_data.iteritems():
            print "key: ", k, "value: ", v

class Interacter_socket():

    def __init__(self, host='', port=''):
        self.host = host
        self.port = port
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.s.bind((self.host, self.port))
        self.conn = None
        self.addr = None

    def listen(self):
        print self.host , ':', self.port, ' is listening'
        # The backlog argument specifies the maximum number of queued connections and should be at least 0;
        # the maximum value is system-dependent (usually 5), the minimum value is forced to 0.
        self.s.listen(1)
        self.conn, self.addr = self.s.accept()
        print('Connected by', self.addr)

    def recv(self):
        # TCP connection is a stream of bytes. But here I assume both side send/read socket fast enough.
        # (since both ns3 and RL takes time and makes enough intervals between send/read)
        # If I find further problem in the future, I will fix it.
        try:
            rcv_str = self.conn.recv(65536)
            if not rcv_str:
                return 'END THIS BATCH', True
            else:
                return rcv_str, False
        except socket.error:
            print "Error while receive from socket"
            self.conn.close()
            return 'Socekt Error', True

    def send(self, string):
        self.conn.sendall(string)

    def close(self):
        try:
            self.conn.close()
        except:
            print "Error while close socket"

if __name__ == "__main__":
    base_port = 12345     # Arbitrary non-privileged port
    batch_count = 0
    while batch_count < 5:

        interacter_socket = Interacter_socket(host = '', port = base_port)
        train_data = Train_data()
        RL = QLearningTable(actions=["use subflow 0", "use subflow 1"])
        interacter_socket.listen()
        recv_str, this_batch_done = interacter_socket.recv()
        train_data.add_one_record(recv_str)

        while True:
            observation_before_action = extract_observation(train_data)
            action = RL.choose_action(observation_before_action)

            print action
            interacter_socket.send(action_translator(train_data, action)) # apply action
            recv_str, this_batch_done = interacter_socket.recv() # get new observation and reward

            if this_batch_done is True:
                break

            train_data.add_one_record(recv_str)
            observation_after_action = extract_observation(train_data)

            reward = calculate_reward(train_data)
            RL.learn(observation_before_action, action, reward, observation_after_action) # RL learning

            observation_before_action = observation_after_action
        interacter_socket.close()
        interacter_socket = None
        print "sleep 30 seconds from now"
        sleep(30)
        batch_count += 1


    #
    # # fresh env
    # env.render()
    #
    # # RL choose action based on observation
    # action = RL.choose_action(str(observation))
    #
    # # RL take action and get next observation and reward
    # observation_, reward, done = env.step(action)
    #
    # # RL learn from this transition
    # RL.learn(str(observation), action, reward, str(observation_))
    #
    # # swap observation
    # observation = observation_
    #
    # # break while loop when end of this episode
    # if done:
    #     break
