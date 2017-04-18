import numpy as np
import pandas as pd
import random
from rl_socket import Interacter_socket
# from rl_server import DataRecorder

def clip(x, low, high):
    return low if x < low else high if x > high else x

class QLearningTable():
    def __init__(self, actions, learning_rate=0.01, reward_decay=0.9, e_greedy=0.90):
        self.actions = actions  # a list
        self.lr = learning_rate
        self.gamma = reward_decay
        self.epsilon = e_greedy
        self.q_table = pd.DataFrame(columns=self.actions)

    def choose_action(self, observation):
        self.check_state_exist(observation)
        # action selection
        if np.random.uniform() < self.epsilon:
            # choose best action
            state_action = self.q_table.ix[observation, :]
            state_action = state_action.reindex(np.random.permutation(state_action.index))     # some actions have same value
            action = state_action.argmax()
        else:
            # choose random action
            action = np.random.choice(self.actions)
        return action

    def learn(self, s, a, r, s_):
        self.check_state_exist(s_)
        q_predict = self.q_table.ix[s, a]
        if s_ != 'terminal':
            q_target = r + self.gamma * self.q_table.ix[s_, :].max()  # next state is not terminal
        else:
            q_target = r  # next state is terminal
        self.q_table.ix[s, a] += self.lr * (q_target - q_predict)  # update

    def check_state_exist(self, state):
        if state not in self.q_table.index:
            # append new state to q table
            self.q_table = self.q_table.append(
                pd.Series(
                    [0]*len(self.actions),
                    index=self.q_table.columns,
                    name=state,
                )
            )

def extract_observation(dataRecorder):
    # dataRecorder:
    #     self.next_seq_num = 0
    #     self.train_data = {}
    #     def add_one_record(self, str_data):
    #     def get_subflow_data_dic(self):
    #     def get_latest_subflow_data(self):
    #     def print_all_train_data(self):
    #     def print_latest_train_data(self):
    value_dic = dataRecorder.get_latest_subflow_data()
    observation = ""
    for i in range(value_dic["nbOfSubflows"]):
        name = "window" + str(i)
        observation += (str(int(value_dic[name]/10)*10) + ':' + name + ',')
    # if value_dic["nbOfSubflows"] == 1:
    #     observation = str(value_dic["window0"])
    # elif value_dic["nbOfSubflows"] == 2:
    #     observation = str(value_dic["window0"] + value_dic["window1"])
    # else:
    #     assert 1 is not 1
    return observation

def action_translator(dataRecorder, action):
    # actions=["use subflow 0", "use subflow 1"]
    last_record = dataRecorder.get_latest_subflow_data()
    if action == "use subflow 0":
        return str(0)
    elif action == "use subflow 1":
        return str(1)
    else:
        print "Error in action_translator"
        assert 1 is not 1

def apply_action(interacter_socket, dataRecorder, action):
    dataRecorder.action.append(action)
    # print dataRecorder.get_latest_subflow_data()
    # print '-- apply action'
    interacter_socket.send(action_translator(dataRecorder, action)) # apply action

def calculate_reward(dataRecorder):
    last_record = dataRecorder.get_latest_subflow_data()
    # print last_record
    # print '--calculate_reward'
    reward = 0
    if "lastAckedSeq1" not in last_record:
        reward = last_record["lastAckedSeq0"]
    else:
        reward = last_record["lastAckedSeq0"] + last_record["lastAckedSeq1"]

    # if dataRecorder.action or dataRecorder.action[-1] != 'not send':
    #     reward -= 10

    return reward
    # don't condiser seq_num wrap
