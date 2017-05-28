import seaborn as sns
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

# data for debug
# df = pd.DataFrame({'Subflow0_Client_Sent': [100000, 100000, 100000, 100000, 100000, 100000,
# 								100000, 100000, 100000, 100000, 100000,100000, 
# 								100000, 100000, 100000, 100000, 100000, 100000,
# 								100000, 100000, 100000, 100000, 100000, 100000,
# 								100000, 100000, 100000, 100000, 100000, 100000,
# 								100000, 100000, 100000, 100000, 100000, 100000],
# 				   'Subflow1_Client_Sent': [200000, 200000, 200000, 200000, 200000, 200000,
# 				   					200000, 200000, 200000, 200000, 200000, 200000,
# 				   					200000, 200000, 200000, 200000, 200000, 200000,
# 				   					200000, 200000, 200000, 200000, 200000, 200000,
# 				   					200000, 200000, 200000, 200000, 200000, 200000,
# 				   					200000, 200000, 200000, 200000, 200000, 200000],
# 				   	'Subflow0_Server_Rcv': [100000, 100000, 100000, 100000, 100000, 100000,
# 								100000, 100000, 100000, 100000, 100000,100000, 
# 								100000, 100000, 100000, 100000, 100000, 100000,
# 								100000, 100000, 100000, 100000, 100000, 100000,
# 								100000, 100000, 100000, 100000, 100000, 100000,
# 								100000, 100000, 100000, 100000, 100000, 100000],
# 				   'Subflow1_Server_Rcv': [300000, 300000, 300000, 300000, 300000, 300000,
# 				   					300000, 300000, 300000, 300000, 300000, 300000,
# 				   					300000, 300000, 300000, 300000, 300000, 300000,
# 				   					300000, 300000, 300000, 300000, 300000, 300000,
# 				   					300000, 300000, 300000, 300000, 300000, 300000,
# 				   					300000, 300000, 300000, 300000, 300000, 300000],
# 				   'Scheduler': ['Round-Robin', 'Fastest-RTT', 'Random', 'Largest-DBP', 'Only Subflow 0', 'Only Subflow 1',
#   				   				'Round-Robin', 'Fastest-RTT', 'Random', 'Largest-DBP', 'Only Subflow 0', 'Only Subflow 1',
#   				   				'Round-Robin', 'Fastest-RTT', 'Random', 'Largest-DBP', 'Only Subflow 0', 'Only Subflow 1',
#   				   				'Round-Robin', 'Fastest-RTT', 'Random', 'Largest-DBP', 'Only Subflow 0', 'Only Subflow 1',
#   				   				'Round-Robin', 'Fastest-RTT', 'Random', 'Largest-DBP', 'Only Subflow 0', 'Only Subflow 1',
# 				   				'Round-Robin', 'Fastest-RTT', 'Random', 'Largest-DBP', 'Only Subflow 0', 'Only Subflow 1'],
# 				   'Experiment': ['Exp 1', 'Exp 1', 'Exp 1', 'Exp 1', 'Exp 1', 'Exp 1',
# 				   				'Exp 2', 'Exp 2', 'Exp 2', 'Exp 2', 'Exp 2', 'Exp 2',
# 				   				'Exp 3', 'Exp 3', 'Exp 3', 'Exp 3', 'Exp 3', 'Exp 3',
# 				   				'Exp 4', 'Exp 4', 'Exp 4', 'Exp 4', 'Exp 4', 'Exp 4',
# 				   				'Exp 5', 'Exp 5', 'Exp 5', 'Exp 5', 'Exp 5', 'Exp 5',
# 				   				'Exp 6', 'Exp 6', 'Exp 6', 'Exp 6', 'Exp 6', 'Exp 6']})

# My design is to put received data into a csv file. Then read dataFrame from csv file.
# By this, record and modification is much easier.
def plotFinalBytes(df)
	df['Total_Client_Sent'] = pd.Series((df['Subflow0_Client_Sent'] + df['Subflow1_Client_Sent']).values, index=df.index)
	df['Total_Server_Rcv'] = pd.Series((df['Subflow0_Server_Rcv'] + df['Subflow1_Server_Rcv']).values, index=df.index)

	# df['Subflow1'] += df['Subflow0']
	sent_df = pd.melt(df, id_vars=['Scheduler', 'Experiment'], value_vars=['Subflow0_Client_Sent', 'Total_Client_Sent'], var_name='SubflowId', value_name='ReceivedBytes')
	# df = pd.melt(df, id_vars=['Scheduler', 'Experiment'], value_vars=['Subflow0', 'SubflowTotal'], var_name='SubflowId', value_name='ReceivedBytes')

	c = ["blue", "red", "green", "pink", "black"]
	for i, g in enumerate(sent_df.groupby("SubflowId")):
		# zorder=-i, so first come plot stay on top
		ax = sns.barplot(data=g[1], x="Experiment", y="ReceivedBytes", hue="Scheduler", color=c[i], zorder=-i, edgecolor="k")

	ax.legend_.remove() # remove the redundant legends 

	sns.plt.savefig("/home/hong/result_figure/sent.png", dpi = 150, bbox_inches='tight')

	# df['Subflow1'] += df['Subflow0']
	rcv_df = pd.melt(df, id_vars=['Scheduler', 'Experiment'], value_vars=['Subflow0_Server_Rcv', 'Total_Server_Rcv'], var_name='SubflowId', value_name='ReceivedBytes')
	# df = pd.melt(df, id_vars=['Scheduler', 'Experiment'], value_vars=['Subflow0', 'SubflowTotal'], var_name='SubflowId', value_name='ReceivedBytes')

	c = ["blue", "red", "green", "pink", "black"]
	for i, g in enumerate(rcv_df.groupby("SubflowId")):
		# zorder=-i, so first come plot stay on top
		ax = sns.barplot(data=g[1], x="Experiment", y="ReceivedBytes", hue="Scheduler", color=c[i], zorder=-i, edgecolor="k")

	ax.legend_.remove() # remove the redundant legends 
	sns.plt.savefig("/home/hong/result_figure/rcv.png", dpi = 150, bbox_inches='tight')
