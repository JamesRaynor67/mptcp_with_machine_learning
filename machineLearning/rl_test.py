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
from RL_core import Actor 
from RL_core import extract_observation
from RL_core import calculate_reward
from RL_core import apply_action
from shutil import copyfile
from rl_server import RecordRTT
from rl_server import RecordCwnd
from rl_server import RecordRwnd
from rl_server import RecordUnAck
from rl_server import RecordAvailableTxBuffer
from rl_server import RecordSchedulerId
from rl_server import RecordMeta
from rl_server import DataRecorder
from rl_server import IsInt

if __name__ == "__main__":

    parser = OptionParser()
    parser.add_option("-m", "--maxEpisode", dest="MaxEpisode", default=1, help="The number of times to train (launch NS3)")
    parser.add_option("-f", "--forceReply", dest="ForceReply", default=None, help="Force RL module reply a scheduler")
    parser.add_option("-i", "--switchInterval", dest="SwitchInterval", default=-1, help="The interval of switching scheduler, note this value maybe better to be the same with the one used in training.")
    parser.add_option("-p", "--savePath", dest="SavePath", default="./rl_training_data/logs/", help="The path to save model and log files")
    parser.add_option("-r", "--storeFile", dest="RestoreFile", default=None, help="The file from which to restore net")
    parser.add_option("-a", "--algorithm", dest="Algorithm", default="DQN", help="The reinforcement learning algorithm to use")
    (options, args) = parser.parse_args()

    assert options.RestoreFile is not None

    episode_count = 0
    if options.Algorithm == "DQN":
        print "Test by DQN"
        RL = DeepQNetwork(n_actions=4, n_features=8, learning_rate=0.01, reward_decay=0.99, e_greedy=0.9, 
                    replace_target_iter=200, memory_size=2000, output_graph=False, save_path=options.SavePath, restore_from_file=options.RestoreFile)
    elif options.Algorithm == "ActorCritic":
        print "Test by ActorCritic"
        sess = tf.Session()
        RL = Actor(sess, n_features=8, n_actions=4, lr=0.001, save_path=options.SavePath, restore_from_file=options.RestoreFile)
    else:
        assert False
        # unimplemented RL algorithm

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
        recv_str, done = socket.recv()

        if done:
            print "RL server ended too early! " + recv_str; exit()

        dataRecorder.add_one_record(recv_str)
        observation = extract_observation(dataRecorder)

        print 'episode: ', episode_count
        lastSchedulerTiming = float("-inf") # float("-inf") ensures that a scheduler is choosen at first time

        while True:
            # Choose action
            # print 'recv_str: ', recv_str
            # print 'observation: ', observation_before_action
            shouldUpdata = False
            if dataRecorder.get_latest_data()["time"] - lastSchedulerTiming > int(options.SwitchInterval): # in microsecond
                lastSchedulerTiming = dataRecorder.get_latest_data()["time"]
                shouldUpdata = True

            if shouldUpdata:
                action = RL.choose_action(observation)
                if options.ForceReply is not None:
                    action = int({"RR":"0", "RTT":"1", "RD":"2", "L-DBP":"3"}[options.ForceReply])
                # print "Hong Jiaming RL: " + str(dataRecorder.get_latest_data()["time"]) + ": is going to use scheduler with id: " + str(action)
                apply_action(socket, dataRecorder, action) # Apply action to environment
            else:
                apply_action(socket, dataRecorder, 999)
            dataRecorder.add_pair_to_last_record(name="schedulerId", value=action)

            recv_str, done = socket.recv() # get new observation and reward
            if done is True:
                break

            dataRecorder.add_one_record(recv_str)
            observation = extract_observation(dataRecorder)

        socket.close()
        socket = None

        copyfile("/home/hong/workspace/mptcp/ns3/mptcp_output/mptcp_client", '/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_mptcp_client')
        copyfile("/home/hong/workspace/mptcp/ns3/mptcp_output/mptcp_drops", '/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_mptcp_drops')
        copyfile("/home/hong/workspace/mptcp/ns3/mptcp_output/mptcp_server", '/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_mptcp_server')
        copyfile("/home/hong/workspace/mptcp/ns3/mptcp_output/mptcp_monitor", '/home/hong/workspace/mptcp/ns3/rl_training_data/' + str(episode_count) + '_mptcp_monitor')

        episode_count += 1
