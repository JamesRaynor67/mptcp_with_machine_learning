import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns
import csv
import sys
import os

def AnalyzeQueueLength(file_path):
    record = []
    with open(file_path, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',')
        next(spamreader)
        for row in spamreader:
            timestamp = (int(row[0])*1.0)/1e9
            queueLenB = int(row[1])
            queueLenC = int(row[2])
            record.append([timestamp, queueLenB, queueLenC])

    columns = ['Timestamp','QueueLenB','QueueLenC']
    queueLen_records = pd.DataFrame(record, columns=columns)

    queueLenB = queueLen_records[["Timestamp", "QueueLenB"]].values
    queueLenC = queueLen_records[["Timestamp", "QueueLenC"]].values

    queueLenB_handle, = sns.plt.plot(list(queueLenB[:,0]), list(queueLenB[:,1]), 'b-')
    queueLenC_handle, = sns.plt.plot(list(queueLenC[:,0]), list(queueLenC[:,1]), 'r-')

    sns.plt.legend([queueLenB_handle, queueLenC_handle], ['Queue length of router D on subflow 0', 'Queue length of router E on subflow 1'], loc='best')
    sns.plt.title('Time-Queue length')
    sns.plt.xlabel('Time / s', fontsize = 14, color = 'black')
    sns.plt.ylabel('Queue length', fontsize = 14, color = 'black')

