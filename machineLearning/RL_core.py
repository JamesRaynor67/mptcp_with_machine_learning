import numpy as np
import pandas as pd
import random

def clip(x, low, high):
    return low if x < low else high if x > high else x

class QLearningTable:
    def __init__(self, actions, learning_rate=0.01, reward_decay=0.5, e_greedy=0.9):
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

def extract_observation(train_data):
    # train_data:
    #     self.next_seq_num = 0
    #     self.train_data = {}
    #     def add_one_record(self, str_data):
    #     def get_train_data_dic(self):
    #     def get_latest_train_data(self):
    #     def print_all_train_data(self):
    #     def print_latest_train_data(self):
    value_dic = train_data.get_latest_train_data()
    observation = ""
    for i in range(value_dic["nbOfSubflows"]):
        name = "window" + str(i)
        observation += (str(value_dic[name]) + ':' + name + ',')
    return observation

def action_translator(train_data, action):
    # actions=["use subflow 0", "use subflow 1"]
    last_record = train_data.get_latest_train_data()
    if action == "use subflow 0":
        return str(0)
    elif action == "use subflow 1":
        return str(1)
    else:
        print "Error in action_translator"
        assert 1 is not 1

def calculate_reward(train_data):
    last_record = train_data.get_latest_train_data()
    # don't condiser seq_num wrap
    if "lastAckedSeq1" not in last_record:
        return last_record["lastAckedSeq0"] - last_record["highTxMark0"]
    else:
        return last_record["lastAckedSeq0"] - last_record["highTxMark0"] + last_record["lastAckedSeq1"] - last_record["highTxMark1"]
