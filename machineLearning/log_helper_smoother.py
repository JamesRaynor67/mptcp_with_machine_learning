import numpy as np
import pandas as pd

def SmoothRate(subflows, groupSize = 10, sampleInteval = 1):
	smoothed_subflows = []
	for subflow in subflows:
		grouped_t = np.convolve(subflow["Timestamp"].values, [1.0/groupSize]*groupSize, 'valid')[::sampleInteval]
		grouped_tx = np.convolve(subflow["TxRate"].values, [1.0/groupSize]*groupSize, 'valid')[::sampleInteval]
		grouped_rx = np.convolve(subflow["RxRate"].values, [1.0/groupSize]*groupSize, 'valid')[::sampleInteval]
		subflowId = np.convolve(subflow["SubflowId"].values, [1.0/groupSize]*groupSize, 'valid')[::sampleInteval] # Easy way to copy subflowId with right length

		# print grouped_t.shape, grouped_tx.shape, grouped_rx.shape, subflowId.shape
		columns = ['Timestamp', 'TxRate', 'RxRate','SubflowId']
		smoothed_subflows.append(pd.DataFrame(np.transpose(np.vstack((grouped_t, grouped_tx, grouped_rx, subflowId))), columns=columns))

	return smoothed_subflows