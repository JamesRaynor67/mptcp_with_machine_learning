import numpy as np
import pandas as pd
import tensorflow as tf
import random
import os
from rl_socket import Interacter_socket

# from rl_server import DataRecorder

def clip(x, low, high):
    return low if x < low else high if x > high else x

#np.random.seed(1)
#tf.set_random_seed(1)

# Deep Q Network off-policy
class DeepQNetwork:
    def __init__(
            self,
            n_actions,
            n_features,
            learning_rate=0.01,
            reward_decay=0.9,
            e_greedy=0.9,
            replace_target_iter=300,
            memory_size=500,
            batch_size=32,
            e_greedy_increment=None,
            output_graph=False,
            save_path=None,
            restore_from_file=None
    ):
        self.n_actions = n_actions
        self.n_features = n_features
        self.lr = learning_rate
        self.gamma = reward_decay
        self.epsilon_max = e_greedy
        self.replace_target_iter = replace_target_iter
        self.memory_size = memory_size
        self.batch_size = batch_size
        self.epsilon_increment = e_greedy_increment
        self.epsilon = 0 if e_greedy_increment is not None else self.epsilon_max

        # total learning step
        self.learn_step_counter = 0

        # initialize zero memory [s, a, r, s_]
        self.memory = np.zeros((self.memory_size, n_features * 2 + 2))
        # consist of [target_net, evaluate_net]

        self.sess = tf.Session()

        self.restore_from_file = restore_from_file
        if self.restore_from_file is None:
            self._build_net()
        else:
            self._restore()

        if output_graph:
            # $ tensorboard --logdir=logs
            self.save_path = save_path
            tf.summary.scalar("loss", self.loss)
            self.merged_summary_op = tf.summary.merge_all()
            self.summaryWriter = tf.summary.FileWriter(self.save_path, self.sess.graph)

        self.sess.run(tf.global_variables_initializer())
        self.cost_his = []

    def _restore(self):
        print "Restore net from file: " + self.restore_from_file
        saver = tf.train.import_meta_graph(self.restore_from_file)
        dirname = os.path.dirname(os.path.abspath(self.restore_from_file))
        saver.restore(self.sess, tf.train.latest_checkpoint(dirname))

        graph = tf.get_default_graph()
        self.s = graph.get_tensor_by_name("s:0")
        self.q_target = graph.get_tensor_by_name("Q_target:0")
        self.q_eval = graph.get_tensor_by_name("eval_net/l2/q_eval:0")
        self.loss = graph.get_tensor_by_name("loss/Mean:0")

    # def _build_net(self):
    #     # ------------------ build evaluate_net ------------------
    #     self.s = tf.placeholder(tf.float32, [None, self.n_features], name='s')  # input
    #     self.q_target = tf.placeholder(tf.float32, [None, self.n_actions], name='Q_target')  # for calculating loss
    #     with tf.variable_scope('eval_net'):
    #         # c_names(collections_names) are the collections to store variables
    #         c_names = ['eval_net_params', tf.GraphKeys.GLOBAL_VARIABLES]
    #         n_l1 = 10 
    #         w_initializer = tf.random_normal_initializer(0., 0.3)
    #         b_initializer = tf.constant_initializer(0.1)

    #         # first layer. collections is used later when assign to target net
    #         with tf.variable_scope('l1'):
    #             w1 = tf.get_variable('w1', [self.n_features, n_l1], initializer=w_initializer, collections=c_names)
    #             b1 = tf.get_variable('b1', [1, n_l1], initializer=b_initializer, collections=c_names)
    #             l1 = tf.nn.relu(tf.matmul(self.s, w1) + b1)

    #         # second layer. collections is used later when assign to target net
    #         with tf.variable_scope('l2'):
    #             w2 = tf.get_variable('w2', [n_l1, self.n_actions], initializer=w_initializer, collections=c_names)
    #             b2 = tf.get_variable('b2', [1, self.n_actions], initializer=b_initializer, collections=c_names)
    #             self.q_eval = tf.add(tf.matmul(l1, w2), b2, name="q_eval")

    #     with tf.variable_scope('loss'):
    #         self.loss = tf.reduce_mean(tf.squared_difference(self.q_target, self.q_eval))
    #     with tf.variable_scope('train'):
    #         self._train_op = tf.train.RMSPropOptimizer(self.lr).minimize(self.loss)

    #     # ------------------ build target_net ------------------
    #     self.s_ = tf.placeholder(tf.float32, [None, self.n_features], name='s_')    # input
    #     with tf.variable_scope('target_net'):
    #         # c_names(collections_names) are the collections to store variables
    #         c_names = ['target_net_params', tf.GraphKeys.GLOBAL_VARIABLES]

    #         # first layer. collections is used later when assign to target net
    #         with tf.variable_scope('l1'):
    #             w1 = tf.get_variable('w1', [self.n_features, n_l1], initializer=w_initializer, collections=c_names)
    #             b1 = tf.get_variable('b1', [1, n_l1], initializer=b_initializer, collections=c_names)
    #             l1 = tf.nn.relu(tf.matmul(self.s_, w1) + b1)

    #         # second layer. collections is used later when assign to target net
    #         with tf.variable_scope('l2'):
    #             w2 = tf.get_variable('w2', [n_l1, self.n_actions], initializer=w_initializer, collections=c_names)
    #             b2 = tf.get_variable('b2', [1, self.n_actions], initializer=b_initializer, collections=c_names)
    #             self.q_next = tf.add(tf.matmul(l1, w2), b2, name="q_next")

    def _build_net(self):
        # ------------------ build evaluate_net ------------------
        self.s = tf.placeholder(tf.float32, [None, self.n_features], name='s')  # input
        self.q_target = tf.placeholder(tf.float32, [None, self.n_actions], name='Q_target')  # for calculating loss
        with tf.variable_scope('eval_net'):
            # c_names(collections_names) are the collections to store variables
            c_names = ['eval_net_params', tf.GraphKeys.GLOBAL_VARIABLES]
            n_l0 = 10
            n_l1 = 8
            w_initializer = tf.random_normal_initializer(0., 0.3)
            b_initializer = tf.constant_initializer(0.1)

            # first layer. collections is used later when assign to target net
            with tf.variable_scope('l0'):
                w0 = tf.get_variable('w0', [self.n_features, n_l0], initializer=w_initializer, collections=c_names)
                b0 = tf.get_variable('b0', [1, n_l0], initializer=b_initializer, collections=c_names)
                l0 = tf.nn.tanh(tf.matmul(self.s, w0) + b0)

            with tf.variable_scope('l1'):
                w1 = tf.get_variable('w1', [n_l0, n_l1], initializer=w_initializer, collections=c_names)
                b1 = tf.get_variable('b1', [1, n_l1], initializer=b_initializer, collections=c_names)
                l1 = tf.nn.relu(tf.matmul(l0, w1) + b1)

            # second layer. collections is used later when assign to target net
            with tf.variable_scope('l2'):
                w2 = tf.get_variable('w2', [n_l1, self.n_actions], initializer=w_initializer, collections=c_names)
                b2 = tf.get_variable('b2', [1, self.n_actions], initializer=b_initializer, collections=c_names)
                self.q_eval = tf.add(tf.matmul(l1, w2), b2, name="q_eval")

        with tf.variable_scope('loss'):
            self.loss = tf.reduce_mean(tf.squared_difference(self.q_target, self.q_eval))
        with tf.variable_scope('train'):
            self._train_op = tf.train.AdamOptimizer(self.lr).minimize(self.loss)

        # ------------------ build target_net ------------------
        self.s_ = tf.placeholder(tf.float32, [None, self.n_features], name='s_')    # input
        with tf.variable_scope('target_net'):
            # c_names(collections_names) are the collections to store variables
            c_names = ['target_net_params', tf.GraphKeys.GLOBAL_VARIABLES]

            # first layer. collections is used later when assign to target net
            with tf.variable_scope('l0'):
                w0 = tf.get_variable('w0', [self.n_features, n_l0], initializer=w_initializer, collections=c_names)
                b0 = tf.get_variable('b0', [1, n_l0], initializer=b_initializer, collections=c_names)
                l0 = tf.nn.tanh(tf.matmul(self.s_, w0) + b0)

            with tf.variable_scope('l1'):
                w1 = tf.get_variable('w1', [n_l0, n_l1], initializer=w_initializer, collections=c_names)
                b1 = tf.get_variable('b1', [1, n_l1], initializer=b_initializer, collections=c_names)
                l1 = tf.nn.relu(tf.matmul(l0, w1) + b1)

            # second layer. collections is used later when assign to target net
            with tf.variable_scope('l2'):
                w2 = tf.get_variable('w2', [n_l1, self.n_actions], initializer=w_initializer, collections=c_names)
                b2 = tf.get_variable('b2', [1, self.n_actions], initializer=b_initializer, collections=c_names)
                self.q_next = tf.add(tf.matmul(l1, w2), b2, name="q_next")

    def store_transition(self, s, a, r, s_):
        if not hasattr(self, 'memory_counter'):
            self.memory_counter = 0

        transition = np.hstack((s, [a, r], s_))

        # replace the old memory with new memory
        index = self.memory_counter % self.memory_size
        self.memory[index, :] = transition

        self.memory_counter += 1

    def choose_action(self, observation):
        # to have batch dimension when feed into tf placeholder
        observation = observation[np.newaxis, :]

        if np.random.uniform() < self.epsilon or self.restore_from_file is not None:
            # forward feed the observation and get q value for every actions
            actions_value = self.sess.run(self.q_eval, feed_dict={self.s: observation})
            action = np.argmax(actions_value)
        else:
            action = np.random.randint(0, self.n_actions)
        return action

    def _replace_target_params(self):
        t_params = tf.get_collection('target_net_params')
        e_params = tf.get_collection('eval_net_params')
        self.sess.run([tf.assign(t, e) for t, e in zip(t_params, e_params)])

    def learn(self):
        # check to replace target parameters
        if self.learn_step_counter % self.replace_target_iter == 0:
            self._replace_target_params()
            print('\ntarget_params_replaced\n')

        # sample batch memory from all memory
        if self.memory_counter > self.memory_size:
            sample_index = np.random.choice(self.memory_size, size=self.batch_size)
        else:
            sample_index = np.random.choice(self.memory_counter, size=self.batch_size)
        batch_memory = self.memory[sample_index, :]

        q_next, q_eval = self.sess.run(
            [self.q_next, self.q_eval],
            feed_dict={
                self.s_: batch_memory[:, -self.n_features:],  # fixed params
                self.s: batch_memory[:, :self.n_features],  # newest params
            })

        # change q_target w.r.t q_eval's action
        q_target = q_eval.copy()

        batch_index = np.arange(self.batch_size, dtype=np.int32)
        eval_act_index = batch_memory[:, self.n_features].astype(int)
        reward = batch_memory[:, self.n_features + 1]

        q_target[batch_index, eval_act_index] = reward + self.gamma * np.max(q_next, axis=1)

        """
        For example in this batch I have 2 samples and 3 actions:
        q_eval =
        [[1, 2, 3],
         [4, 5, 6]]

        q_target = q_eval =
        [[1, 2, 3],
         [4, 5, 6]]

        Then change q_target with the real q_target value w.r.t the q_eval's action.
        For example in:
            sample 0, I took action 0, and the max q_target value is -1;
            sample 1, I took action 2, and the max q_target value is -2:
        q_target =
        [[-1, 2, 3],
         [4, 5, -2]]

        So the (q_target - q_eval) becomes:
        [[(-1)-(1), 0, 0],
         [0, 0, (-2)-(6)]]

        We then backpropagate this error w.r.t the corresponding action to network,
        leave other action as error=0 cause we didn't choose it.
        """

        # train eval network
        _, self.cost, summary_str = self.sess.run([self._train_op, self.loss, self.merged_summary_op],
                                     feed_dict={self.s: batch_memory[:, :self.n_features],
                                                self.q_target: q_target})
        self.cost_his.append(self.cost)
        self.summaryWriter.add_summary(summary_str, self.learn_step_counter)

        # increasing epsilon
        self.epsilon = self.epsilon + self.epsilon_increment if self.epsilon < self.epsilon_max else self.epsilon_max
        self.learn_step_counter += 1

    def save_model(self, isFianl=False):
        saver = tf.train.Saver()
        if isFianl is False:
            saver.save(self.sess, os.path.join(self.save_path, 'my_model'), global_step=self.learn_step_counter)
        else:
            saver.save(self.sess, os.path.join(self.save_path, 'my_final_model'), global_step=self.learn_step_counter)

    def plot_cost(self):
        import matplotlib.pyplot as plt
        plt.plot(np.arange(len(self.cost_his)), self.cost_his)
        plt.ylabel('Cost')
        plt.xlabel('training steps')
        plt.show()

class Actor(object):
    def __init__(self, sess, n_features, n_actions, lr=0.001, save_path=None, restore_from_file=None):
        self.sess = sess
        self.restore_from_file = restore_from_file
        self.save_path = save_path
        self.n_features = n_features
        self.n_actions = n_actions
        self.lr = lr
        if self.restore_from_file is None:
            self._build_net()
        else:
            self._restore()

    def _build_net(self):
        self.s = tf.placeholder(tf.float32, [1, self.n_features], "state")
        self.a = tf.placeholder(tf.int32, None, "act")
        self.td_error = tf.placeholder(tf.float32, None, "td_error")  # TD_error
        n_l1 = 10
        with tf.variable_scope('Actor'):
            with tf.variable_scope('l1'):
                w1 = tf.get_variable('w1', [self.n_features, n_l1], initializer=tf.random_normal_initializer(0.0, 0.1))
                b1 = tf.get_variable('b1', [1, n_l1], initializer=tf.constant_initializer(0.1))
                l1 = tf.nn.tanh(tf.matmul(self.s, w1) + b1)
            with tf.variable_scope('l2'):
                w2 = tf.get_variable('w2', [n_l1, self.n_actions], initializer=tf.random_normal_initializer(0.0, 0.1))
                b2 = tf.get_variable('b2', [1, self.n_actions], initializer=tf.constant_initializer(0.1))
                self.acts_prob = tf.nn.softmax(tf.add(tf.matmul(l1, w2), b2), name="acts_prob")
        with tf.variable_scope('exp_v'):
            log_prob = tf.log(self.acts_prob[0, self.a])
            self.exp_v = tf.reduce_mean(log_prob * self.td_error)  # advantage (TD_error) guided loss
        with tf.variable_scope('train'):
            self.train_op = tf.train.AdamOptimizer(self.lr).minimize(-self.exp_v)  # minimize(-exp_v) = maximize(exp_v)

    def learn(self, s, a, td):
        s = s[np.newaxis, :]
        feed_dict = {self.s: s, self.a: a, self.td_error: td}
        _, exp_v = self.sess.run([self.train_op, self.exp_v], feed_dict)
        return exp_v

    def choose_action(self, s):
        s = s[np.newaxis, :]
        probs = self.sess.run(self.acts_prob, {self.s: s})   # get probabilities for all actions
        return np.random.choice(np.arange(probs.shape[1]), p=probs.ravel())   # return a int

    def _restore(self):
        print "Restore net from file: " + self.restore_from_file
        saver = tf.train.import_meta_graph(self.restore_from_file)
        dirname = os.path.dirname(os.path.abspath(self.restore_from_file))
        saver.restore(self.sess, tf.train.latest_checkpoint(dirname))

        graph = tf.get_default_graph()
        self.s = graph.get_tensor_by_name("state:0")
        self.acts_prob = graph.get_tensor_by_name("Actor/l2/acts_prob:0")
        self.a = graph.get_tensor_by_name("act:0")
        self.td_error = graph.get_tensor_by_name("td_error:0")

    def save_model(self, step, isFianl=False):
        saver = tf.train.Saver()
        if isFianl is False:
            saver.save(self.sess, os.path.join(self.save_path, 'my_model'), global_step=step)
        else:
            saver.save(self.sess, os.path.join(self.save_path, 'my_final_model'), global_step=step)

class Critic(object):
    def __init__(self, sess, n_features, lr=0.01, save_path=None, restore_from_file=None):
        self.sess = sess
        self.s = tf.placeholder(tf.float32, [1, n_features], "state")
        self.v_ = tf.placeholder(tf.float32, [1, 1], "v_next")
        self.r = tf.placeholder(tf.float32, None, 'r')
        n_l1 = 10
        GAMMA = 0.9
        with tf.variable_scope('Critic'):
            with tf.variable_scope('l1'):
                w1 = tf.get_variable('w1', [n_features, n_l1], initializer=tf.random_normal_initializer(0.0, 0.1))
                b1 = tf.get_variable('b1', [1, n_l1], initializer=tf.constant_initializer(0.1))
                l1 = tf.nn.relu(tf.matmul(self.s, w1) + b1)

            with tf.variable_scope('l2'):
                w2 = tf.get_variable('w2', [n_l1, 1], initializer=tf.random_normal_initializer(0.0, 0.1))
                b2 = tf.get_variable('b2', [1, 1], initializer=tf.constant_initializer(0.1))
                self.v = tf.add(tf.matmul(l1, w2), b2, name="V")

        with tf.variable_scope('squared_TD_error'):
            self.td_error = self.r + GAMMA * self.v_ - self.v
            self.loss = tf.square(self.td_error)    # TD_error = (r+gamma*V_next) - V_eval
        with tf.variable_scope('train'):
            self.train_op = tf.train.AdamOptimizer(lr).minimize(self.loss)

    def learn(self, s, r, s_):
        s, s_ = s[np.newaxis, :], s_[np.newaxis, :]

        v_ = self.sess.run(self.v, {self.s: s_})
        td_error, _ = self.sess.run([self.td_error, self.train_op],
                                          {self.s: s, self.v_: v_, self.r: r})
        return td_error

    def _restore(self):
        assert False
        # There is no need for Critic to restore. I only use Actor when test

    def save_model(self, isFianl=False):
        assert False
        # Let's save model file in Actor

def extract_observation(dataRecorder):
    # let's set return value "observation" to be a np array with fixed length
    # value_dic = dataRecorder.get_latest_data()
    # observation = np.zeros((4))
    # for i in range(value_dic["nbOfSubflows"]):
    #     name = "window" + str(i)
    #     observation[i*2] = value_dic[name]
    #     name = "cWnd" + str(i)
    #     observation[i*2+1] = value_dic[name]
    # return observation
    value_dic = dataRecorder.get_latest_data()
    observation = np.zeros((8))
    for i in range(value_dic["nbOfSubflows"]):
        name = "window" + str(i)
        observation[i*4] = value_dic[name]
        name = "cWnd" + str(i)
        observation[i*4+1] = value_dic[name]
        name = "rtt" + str(i)
        observation[i*4+2] = value_dic[name]
        name = "availableTxBuffer" + str(i)
        observation[i*4+3] = value_dic[name]
    return observation / 1e6 # make it close to 1 to help training

def action_translator(dataRecorder, action):
    # action is a numpy array, so we need translator
    # let's set return value "action" to be an integer
    # 0 choose RR
    # 1 choose RTT
    # 2 choose RD
    # 3 choose L-DBP
    # print type(action), action
    return str(int(action))

def apply_action(interacter_socket, dataRecorder, action):
    dataRecorder.action.append(action)
    tx_str = action_translator(dataRecorder, action)
    interacter_socket.send(tx_str) # apply action


def calculate_reward(dataRecorder, reset = False, byAck=True):
    if byAck is True:
        if reset == False:
            last_record = dataRecorder.get_latest_data()
            # print last_record
            # print '--calculate_reward'
            # print 'old lastAckedSeqSum == ', calculate_reward.lastAckedSeqSum
            reward = 0
            for i in range(last_record['nbOfSubflows']):
                reward += last_record["lastAckedSeq" + str(i)]
            reward -= calculate_reward.lastAckedSeqSum

            calculate_reward.lastAckedSeqSum = 0
            for i in range(last_record['nbOfSubflows']):
                calculate_reward.lastAckedSeqSum += last_record["lastAckedSeq" + str(i)]

            # if dataRecorder.action or dataRecorder.action[-1] != 'not send':
            #     reward -= 10
            # print 'new lastAckedSeqSum == ', calculate_reward.lastAckedSeqSum
            # print 'reward == ' + str(reward)
            if reward == 0:
                return -0.1
            else:
                return reward / 1500.0
            # don't condiser seq_num wrap
        else:
            # print 'In last episode, calculate_reward.lastAckedSeqSum == ', calculate_reward.lastAckedSeqSum
            calculate_reward.lastAckedSeqSum = 0
            return 0
    else:
        assert False # Disable is part temporaly
        if reset == False:
            last_record = dataRecorder.get_latest_data()

            reward = 0
            for i in range(last_record['nbOfSubflows']):
                reward += last_record["highTxMark" + str(i)]
            reward -= calculate_reward.highTxSeqSum

            calculate_reward.highTxSeqSum = 0
            for i in range(last_record['nbOfSubflows']):
                calculate_reward.highTxSeqSum += last_record["highTxMark" + str(i)]

            return reward / 1500.0
            # don't condiser seq_num wrap
        else:
            calculate_reward.highTxSeqSum = 0
            return 0

calculate_reward.lastAckedSeqSum = 0
calculate_reward.highTxSeqSum = 0
