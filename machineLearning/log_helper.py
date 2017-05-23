import numpy as np
import pandas as pd

def ConvertByteToBit(l):
	assert type(l) is np.ndarray and type(l[0]) is not list
	return l * 8

def GetRate(subflows):
	subflow_rates = []
	for subflow in subflows:
		# Rate is in unit of bps
		tRate = ConvertByteToBit(np.diff(subflow["TxBytes"].values)/np.diff(subflow["Timestamp"].values))
		rRate = ConvertByteToBit(np.diff(subflow["RxBytes"].values)/np.diff(subflow["Timestamp"].values))
		timestamp = subflow["Timestamp"].values[:-1]
		subflowId = np.ones((subflow["SubflowId"].shape[0]-1)) * subflow["SubflowId"].values[0]
		print tRate.shape, rRate.shape, timestamp.shape, subflowId.shape
		columns = ['Timestamp','TxRate','RxRate','SubflowId']
		subflow_rates.append(pd.DataFrame(np.transpose(np.vstack((timestamp, tRate, rRate, subflowId))), columns=columns))
	return subflow_rates