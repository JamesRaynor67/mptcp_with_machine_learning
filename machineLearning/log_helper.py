import numpy as np

def ConvertByteToBit(l):
	assert type(l) is np.ndarray and type(l[0]) is not list
	return [ele * 8 for ele in l]