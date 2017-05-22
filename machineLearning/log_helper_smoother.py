import numpy as np

def Smooth(t, x, y):
	assert len(t)-1 == len(x) and len(x) == len(y) and (t[0] is not list) \
			and (x[0] is not list) and (y[0] is not list)

	groupSize = 60
	sampleInteval = 10
	grouped_x = np.convolve(x, [1.0/groupSize]*groupSize, 'valid')[::sampleInteval]
	grouped_y = np.convolve(y, [1.0/groupSize]*groupSize, 'valid')[::sampleInteval]
	grouped_t = np.convolve(t[0:-1], [1.0/groupSize]*groupSize, 'valid')[::sampleInteval]
	return grouped_t, grouped_x, grouped_y