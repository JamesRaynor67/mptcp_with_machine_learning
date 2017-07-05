import socket
import pandas
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
import tensorflow as tf
from time import sleep
from optparse import OptionParser

from rl_socket import Interacter_socket
from RL_core import DeepQNetwork
from RL_core import extract_observation
from RL_core import calculate_reward
from RL_core import apply_action
from RL_core import Actor
from RL_core import Critic
from shutil import copyfile

class RecordRTT():
    def __init__(self, path):
        self.f = open(path, 'w')
        self.f.write("timestamp,rtt0,rtt1\n")

    def addRTTRecord(self, one_row):
        rtt0 = -1;
        rtt1 = -1;
        if('rtt0' in one_row):
            rtt0 = int(one_row['rtt0'])
        if('rtt1' in one_row):
            rtt1 = int(one_row['rtt1'])
        self.f.write(str(one_row['time']) + ',' + str(rtt0) + ',' + str(rtt1) + '\n')
        assert ('rtt2' not in one_row)

    def __del__(self):
        if not self.f.closed:
            self.f.close()

class RecordCwnd():
    def __init__(self, path):
        self.f = open(path, 'w')
        self.f.write("timestamp,cWnd0,cWnd1\n")

    def addCwndRecord(self, one_row):
        cWnd0 = -1;
        cWnd1 = -1;
        if('cWnd0' in one_row):
            cWnd0 = int(one_row['cWnd0'])
        if('cWnd1' in one_row):
            cWnd1 = int(one_row['cWnd1'])
        self.f.write(str(one_row['time']) + ',' + str(cWnd0) + ',' + str(cWnd1) + '\n')
        assert ('cWnd2' not in one_row)

    def __del__(self):
        if not self.f.closed:
            self.f.close()

class RecordRwnd():
    def __init__(self, path):
        self.f = open(path, 'w')
        self.f.write("timestamp,rWnd0,rWnd1\n")

    def addRwndRecord(self, one_row):
        rWnd0 = -1;
        rWnd1 = -1;
        if('rWnd0' in one_row):
            rWnd0 = int(one_row['rWnd0'])
        if('rWnd1' in one_row):
            rWnd1 = int(one_row['rWnd1'])
        self.f.write(str(one_row['time']) + ',' + str(rWnd0) + ',' + str(rWnd1) + '\n')
        assert ('rWnd2' not in one_row)

    def __del__(self):
        if not self.f.closed:
            self.f.close()

class RecordUnAck():
    def __init__(self, path):
        self.f = open(path, 'w')
        self.f.write("timestamp,unAck0,unAck1\n")

    def addUnAckRecord(self, one_row):
        unAck0 = -1;
        unAck1 = -1;
        if('unAck0' in one_row):
            unAck0 = int(one_row['unAck0'])
        if('unAck1' in one_row):
            unAck1 = int(one_row['unAck1'])
        self.f.write(str(one_row['time']) + ',' + str(unAck0) + ',' + str(unAck1) + '\n')
        assert ('unAck2' not in one_row)

    def __del__(self):
        if not self.f.closed:
            self.f.close()

class RecordAvailableTxBuffer():
    def __init__(self, path):
        self.f = open(path, 'w')
        self.f.write("timestamp,availableTxBuffer0,availableTxBuffer1\n")

    def addTxBufferRecord(self, one_row):
        availableTxBuffer0 = -1;
        availableTxBuffer1 = -1;
        if('availableTxBuffer0' in one_row):
            availableTxBuffer0 = int(one_row['availableTxBuffer0'])
        if('availableTxBuffer1' in one_row):
            availableTxBuffer1 = int(one_row['availableTxBuffer1'])
        self.f.write(str(one_row['time']) + ',' + str(availableTxBuffer0) + ',' + str(availableTxBuffer1) + '\n')
        assert ('availableTxBuffer2' not in one_row)

    def __del__(self):
        if not self.f.closed:
            self.f.close()

class RecordSchedulerId():
    def __init__(self, path):
        self.f = open(path, 'w')
        self.f.write("timestamp,schedulerId\n")

    def addSchedulerId(self, timeStamp, schedulerId):
        self.f.write(str(timeStamp) + ',' + str(schedulerId) + '\n')

    def __del__(self):
        if not self.f.closed:
            self.f.close()

class RecordMeta():
    def __init__(self, path):
        self.f = open(path, 'w')
        self.f.write("timestamp,lastAckedSeq,highTxMark,availableTxBuffer,nextTxSeq,totalCwnd,unAckMeta\n")

    def addMetaRecord(self, one_row):
        time = str(one_row['time'])
        lastAckedSeq = str(one_row['lastAckedSeqMeta'])
        highTxMark = str(one_row['highTxMarkMeta'])
        availableTxBuffer = str(one_row['availableTxBufferMeta'])
        nextTxSeq = str(one_row['nextTxSeqMeta'])
        totalCwnd = str(one_row['totalCwndMeta'])
        unAck = str(one_row['unAckMeta'])
        self.f.write(time + ',' + lastAckedSeq + ',' + highTxMark + ',' + availableTxBuffer + ',' + nextTxSeq + ',' + totalCwnd + ',' + unAck + '\n')

    def __del__(self):
        if not self.f.closed:
            self.f.close()

def IsInt(s):
    # A naive method, but enough here
    if "." in s:
        return False
    else:
        return True

class DataRecorder():

    def __init__(self, rttRecord, cWndRecord, rWndRecord, unAckRecord, availableTxBufferRecord, schedulerIdRecord, metaRecord):
        self.next_seq_num = 0
        self.data = {}
        self.action = []
        self.rttRecord = rttRecord
        self.cWndRecord = cWndRecord
        self.rWndRecord = rWndRecord
        self.unAckRecord = unAckRecord
        self.availableTxBufferRecord = availableTxBufferRecord
        self.metaRecord = metaRecord
        self.schedulerIdRecord = schedulerIdRecord

    def add_one_record(self, str_data):
        # global g_TcWnd0
        # global g_TcWnd1
        # name#value$name$value...
        pair_list = str_data.split("$")
        one_row = {}
        for pair in pair_list:
            if len(pair) > 3: # this ensures the string (pair) is not empty or string with single '$'
                name_val_list = pair.split("#")
                # print "Hong Jiaming: 1 ", pair, len(name_val_list), name_val_list[0], name_val_list[1]
                if IsInt(name_val_list[1]):
                    one_row[name_val_list[0]] = int(name_val_list[1])
                else:
                    one_row[name_val_list[0]] = float(name_val_list[1])

        # ensure this transmission is right and complete
        # neighbour TCP segments must not combined into one
        assert one_row["size"] == len(one_row)
        assert one_row["ssn"] == self.next_seq_num
        self.rttRecord.addRTTRecord(one_row)
        self.cWndRecord.addCwndRecord(one_row)
        self.rWndRecord.addRwndRecord(one_row)
        self.unAckRecord.addUnAckRecord(one_row)
        self.availableTxBufferRecord.addTxBufferRecord(one_row)
        self.metaRecord.addMetaRecord(one_row)
        self.data[self.next_seq_num] = one_row
        self.next_seq_num += 1          

    def get_data_dic(self):
        return self.data

    def get_latest_data(self):
        return self.data[self.next_seq_num-1]

    def add_pair_to_last_record(self, name, value):
        self.data[self.next_seq_num-1][name] = value
        if name is "schedulerId":
            self.schedulerIdRecord.addSchedulerId(timeStamp=self.data[self.next_seq_num-1]["time"], schedulerId=value)

    def print_all_data(self):
        print "dic size: ", len(self.data)
        for ssn, data in self.data.iteritems():
            for k, v in data.iteritems():
                print "key: ", k, "value: ", v

    def print_latest_data(self):
        latest_data = self.data[self.next_seq_num-1]
        for k, v in latest_data.iteritems():
            print "key: ", k, "value: ", v

if __name__ == "__main__":

    parser = OptionParser()
    parser.add_option("-f", "--forceReply", dest="ForceReply", default=None, help="Force RL module reply a scheduler")
    parser.add_option("-m", "--maxEpisode", dest="MaxEpisode", default=1, help="The number of times to train (launch NS3)")
    parser.add_option("-i", "--switchInterval", dest="SwitchInterval", default=-1, help="The interval of switching scheduler")
    parser.add_option("-p", "--savePath", dest="SavePath", default="./rl_training_data/logs/", help="The path to save model and log files")
    parser.add_option("-a", "--algorithm", dest="Algorithm", default="DQN", help="The reinforcement learning algorithm to use")
    parser.add_option("-r", "--rewardByAck", dest="RewardByAck", default=True, help="Reward is calculated by lastAckedSeqNum")
    (options, args) = parser.parse_args()

    if options.Algorithm == "DQN":
        print "Training by DQN"
        episode_count = 0
        RL = DeepQNetwork(n_actions=4, n_features=8, learning_rate=0.01, reward_decay=0.99, e_greedy=0.9, 
                    replace_target_iter=200, memory_size=2000, output_graph=True, save_path=options.SavePath, restore_from_file=None)
        while episode_count < int(options.MaxEpisode):
            rttRecorder = RecordRTT('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_client_rtt')
            cWndRecorder = RecordCwnd('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_client_cWnd')
            rWndRecorder = RecordRwnd('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_client_rWnd')
            unAckRecorder = RecordUnAck('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_client_unAck')
            availableTxBufferRecord = RecordAvailableTxBuffer('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_client_txBufferSize')
            metaRecorder = RecordMeta('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_meta_socket')
            schedulerIdRecord = RecordSchedulerId('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_schedulerId')
            dataRecorder = DataRecorder(rttRecorder, cWndRecorder, rWndRecorder, unAckRecorder, availableTxBufferRecord, schedulerIdRecord, metaRecorder)

            socket = Interacter_socket(host = '', port = 12345)
            socket.listen()
            recv_str, this_episode_done = socket.recv()

            if this_episode_done:
                print "RL server ended too early! " + recv_str; exit()

            dataRecorder.add_one_record(recv_str)
            observation_before_action = extract_observation(dataRecorder)
            reward = calculate_reward(dataRecorder, reset = True, byAck=options.RewardByAck)

            print 'episode: ', episode_count
            f = open("/home/hong/workspace/mptcp/ns3/mptcp_output/calculate_reward", 'w'); f.write("time,reward\n")
            step, lastSchedulerTiming, accumulativeReward = 0, float("-inf"), 0 # float("-inf") ensures that a scheduler is choosen at first time

            while True:
                # Choose action
                # print 'recv_str: ', recv_str
                # print 'observation: ', observation_before_action
                shouldUpdata = False
                if dataRecorder.get_latest_data()["time"] - lastSchedulerTiming > int(options.SwitchInterval): # in microsecond
                    lastSchedulerTiming = dataRecorder.get_latest_data()["time"]
                    shouldUpdata = True
                    accumulativeReward = 0

                if shouldUpdata:
                    action = RL.choose_action(observation_before_action)
                    if options.ForceReply is not None:
                        action = int({"RR":"0", "RTT":"1", "RD":"2", "L-DBP":"3"}[options.ForceReply])
                        # action = int({"L-DBP":"0", "RD":"1", "RR":"2", "RTT":"3"}[options.ForceReply])
                    # print "Hong Jiaming RL: " + str(dataRecorder.get_latest_data()["time"]) + ": is going to use scheduler with id: " + str(action)
                    apply_action(socket, dataRecorder, action) # Apply action to environment
                else:
                    apply_action(socket, dataRecorder, 999)
                dataRecorder.add_pair_to_last_record(name="schedulerId", value=action)

                recv_str, this_episode_done = socket.recv() # get new observation and reward
                if this_episode_done is True:
                    break

                dataRecorder.add_one_record(recv_str)
                observation_after_action = extract_observation(dataRecorder)
                reward = calculate_reward(dataRecorder, reset = False, byAck=options.RewardByAck)
                accumulativeReward += reward
                # # Update memory
                RL.store_transition(observation_before_action, action, accumulativeReward, observation_after_action)

                if (step > 200) and (step % 5 == 0):   # may need other parameters?
                    RL.learn()

                observation_before_action = observation_after_action

                f.write(str(dataRecorder.get_latest_data()["time"]) + ',' + str(reward) + '\n')
                step += 1

            if episode_count % 50 == 0:
                RL.save_model(isFianl=False)

            socket.close()
            socket = None
            f.close()

            copyfile("/home/hong/workspace/mptcp/ns3/mptcp_output/calculate_reward", '/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_calculate_reward')
            copyfile("/home/hong/workspace/mptcp/ns3/mptcp_output/mptcp_client", '/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_mptcp_client')
            copyfile("/home/hong/workspace/mptcp/ns3/mptcp_output/mptcp_drops", '/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_mptcp_drops')
            copyfile("/home/hong/workspace/mptcp/ns3/mptcp_output/mptcp_server", '/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_mptcp_server')
            copyfile("/home/hong/workspace/mptcp/ns3/mptcp_output/mptcp_monitor", '/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_mptcp_monitor')
            copyfile("/home/hong/workspace/mptcp/ns3/mptcp_output/routers_queue_len", '/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_routers_queue_len')
            episode_count += 1
        RL.save_model(isFianl=True)
    
    elif options.Algorithm == "ActorCritic":
        print "Training by ActorCritic"
        episode_count = 0
        sess = tf.Session()
        actor = Actor(sess, n_features=8, n_actions=4, lr=0.001, save_path=options.SavePath, restore_from_file=None)
        critic = Critic(sess, n_features=8, lr=0.01)     # we need a good teacher, so the teacher should learn faster than the actor
        sess.run(tf.global_variables_initializer())

        while episode_count < int(options.MaxEpisode):
            rttRecorder = RecordRTT('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_client_rtt')
            cWndRecorder = RecordCwnd('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_client_cWnd')
            rWndRecorder = RecordRwnd('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_client_rWnd')
            unAckRecorder = RecordUnAck('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_client_unAck')
            availableTxBufferRecord = RecordAvailableTxBuffer('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_client_txBufferSize')
            metaRecorder = RecordMeta('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_meta_socket')
            schedulerIdRecord = RecordSchedulerId('/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_schedulerId')
            dataRecorder = DataRecorder(rttRecorder, cWndRecorder, rWndRecorder, unAckRecorder, availableTxBufferRecord, schedulerIdRecord, metaRecorder)
            socket = Interacter_socket(host = '', port = 12345)
            socket.listen()
            recv_str, this_episode_done = socket.recv()

            if this_episode_done:
                print "RL server ended too early! " + recv_str; exit()

            dataRecorder.add_one_record(recv_str)
            observation_before_action = extract_observation(dataRecorder)
            reward = calculate_reward(dataRecorder, reset = True, byAck=options.RewardByAck)

            print 'episode: ', episode_count
            f = open("/home/hong/workspace/mptcp/ns3/mptcp_output/calculate_reward", 'w'); f.write("time,reward\n")
            step, lastSchedulerTiming, accumulativeReward = 0, float("-inf"), 0 # float("-inf") ensures that a scheduler is choosen at first time

            while True:
                # Choose action
                # print 'recv_str: ', recv_str
                # print 'observation: ', observation_before_action
                shouldUpdata = False
                if dataRecorder.get_latest_data()["time"] - lastSchedulerTiming > int(options.SwitchInterval): # in microsecond
                    lastSchedulerTiming = dataRecorder.get_latest_data()["time"]
                    shouldUpdata = True

                if shouldUpdata:
                    action = actor.choose_action(observation_before_action)
                    if options.ForceReply is not None:
                        action = int({"RR":"0", "RTT":"1", "RD":"2", "L-DBP":"3"}[options.ForceReply])
                    # print "Hong Jiaming RL: " + str(dataRecorder.get_latest_data()["time"]) + ": is going to use scheduler with id: " + str(action)
                    apply_action(socket, dataRecorder, action) # Apply action to environment
                else:
                    apply_action(socket, dataRecorder, 999)
                dataRecorder.add_pair_to_last_record(name="schedulerId", value=action)

                recv_str, this_episode_done = socket.recv() # get new observation and reward
                if this_episode_done is True:
                    break

                dataRecorder.add_one_record(recv_str)
                observation_after_action = extract_observation(dataRecorder)            
                reward = calculate_reward(dataRecorder, reset = False, byAck=options.RewardByAck)
                td_error = critic.learn(observation_before_action, reward, observation_after_action)
                actor.learn(observation_before_action, action, td_error)

                observation_before_action = observation_after_action

                f.write(str(dataRecorder.get_latest_data()["time"]) + ',' + str(reward) + '\n')
                step += 1

            if episode_count % 50 == 0:
                actor.save_model(step=step, isFianl=False)

            socket.close()
            socket = None
            f.close()

            copyfile("/home/hong/workspace/mptcp/ns3/mptcp_output/calculate_reward", '/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_calculate_reward')
            copyfile("/home/hong/workspace/mptcp/ns3/mptcp_output/mptcp_client", '/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_mptcp_client')
            copyfile("/home/hong/workspace/mptcp/ns3/mptcp_output/mptcp_drops", '/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_mptcp_drops')
            copyfile("/home/hong/workspace/mptcp/ns3/mptcp_output/mptcp_server", '/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_mptcp_server')
            copyfile("/home/hong/workspace/mptcp/ns3/mptcp_output/mptcp_monitor", '/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_mptcp_monitor')
            copyfile("/home/hong/workspace/mptcp/ns3/mptcp_output/routers_queue_len", '/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_routers_queue_len')
            episode_count += 1
        actor.save_model(step=step, isFianl=True)
    else:
        print "Unknow RL algorithm"
        assert False