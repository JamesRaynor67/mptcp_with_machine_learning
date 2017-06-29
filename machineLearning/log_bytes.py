import seaborn as sns
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

# My design is to put received data into a csv file. Then read dataFrame from csv file.
# By this, record and modification is much easier.
def plotThroughputs(path):
	df = pd.read_csv(path)
	df['Scheduler'] = pd.Categorical(df['Scheduler'], ['L-DBP', 'RD', 'RR', 'RTT', 'RL-Choose'])
	df = df.sort_values(by=['Experiment', 'Scheduler'])

	df['Total_Client_Sent'] = pd.Series((df['Subflow0_Client_Sent'] + df['Subflow1_Client_Sent']).values, index=df.index)
	df['Total_Server_Rcv'] = pd.Series((df['Subflow0_Server_Rcv'] + df['Subflow1_Server_Rcv']).values, index=df.index)

	# df['Subflow1'] += df['Subflow0']
	sns.plt.figure(figsize=(16*1.5, 9*1.5))
	sent_df = pd.melt(df, id_vars=['Scheduler', 'Experiment'], value_vars=['Subflow0_Client_Sent', 'Total_Client_Sent'], var_name='SubflowId', value_name='ReceivedBytes')
	# df = pd.melt(df, id_vars=['Scheduler', 'Experiment'], value_vars=['Subflow0', 'SubflowTotal'], var_name='SubflowId', value_name='ReceivedBytes')

	c = ["blue", "red", "green", "pink", "black"]
	for i, g in enumerate(sent_df.groupby("SubflowId")):
		# zorder=-i, so first come plot stay on top
		ax = sns.barplot(data=g[1], x="Experiment", y="ReceivedBytes", hue="Scheduler", color=c[i], zorder=-i, edgecolor="k")

	# ax.legend_.remove() # remove the redundant legends 

	# sns.plt.savefig("/home/hong/result_figure/sent.png", dpi = 150, bbox_inches='tight')
	plt.legend(bbox_to_anchor=(1.05, 1), loc=2, borderaxespad=0.)
	sns.plt.savefig("/home/hong/result_figure/sent.png", dpi = 150)
	sns.plt.close()

	sns.plt.figure(figsize=(16*1.5, 9*1.5))
	# df['Subflow1'] += df['Subflow0']
	rcv_df = pd.melt(df, id_vars=['Scheduler', 'Experiment'], value_vars=['Subflow0_Server_Rcv', 'Total_Server_Rcv'], var_name='SubflowId', value_name='ReceivedBytes')
	# df = pd.melt(df, id_vars=['Scheduler', 'Experiment'], value_vars=['Subflow0', 'SubflowTotal'], var_name='SubflowId', value_name='ReceivedBytes')

	c = ["blue", "red", "green", "pink", "black"]
	for i, g in enumerate(rcv_df.groupby("SubflowId")):
		# zorder=-i, so first come plot stay on top
		ax = sns.barplot(data=g[1], x="Experiment", y="ReceivedBytes", hue="Scheduler", color=c[i], zorder=-i, edgecolor="k")

	# ax.legend_.remove() # remove the redundant legends 
	# sns.plt.savefig("/home/hong/result_figure/rcv.png", dpi = 150, bbox_inches='tight')
	plt.legend(bbox_to_anchor=(1.05, 1), loc=2, borderaxespad=0.)
	sns.plt.savefig("/home/hong/result_figure/rcv.png", dpi = 150)
	return df

def plotThroughputSummary(df):
	sns.plt.figure(figsize=(16*1.5, 9*1.5))
	# Takes out total received throughput in each experiment of each scheduler
	rcv_df = pd.melt(df, id_vars=['Scheduler', 'Experiment'], value_vars=['Subflow0_Server_Rcv', 'Total_Server_Rcv'], var_name='SubflowId', value_name='ReceivedBytes')
	rcv_df = rcv_df[rcv_df['SubflowId'].isin(['Total_Server_Rcv'])]

	# Sum up the throughput of each experiment by scheduler
	scheduler_sum_df = rcv_df.groupby(by=['Scheduler'], as_index=False)['ReceivedBytes'].sum()
	scheduler_sum_df['Scheduler'] = pd.Categorical(scheduler_sum_df['Scheduler'], ['L-DBP', 'RD', 'RR', 'RTT', 'RL-Choose'])
	scheduler_sum_df = scheduler_sum_df.sort_values(by=['Scheduler'])
	scheduler_sum_df = scheduler_sum_df.set_index(['Scheduler'])
	scheduler_sum_df['Scheduler'] = scheduler_sum_df.index

	# Calculate the right value of RL-Choose
	scheduler_sum_df.set_value('RL-Choose', 'ReceivedBytes', scheduler_sum_df.loc['RL-Choose']['ReceivedBytes']/3) # NOTE: devide by 3 is that RL-Choose is experimented three times

	g = sns.factorplot(data=scheduler_sum_df, x="Scheduler", y="ReceivedBytes", kind="bar", palette=sns.light_palette("red"))
	sns.plt.savefig("/home/hong/result_figure/rcv_sum_by_scheduler.png", dpi = 150)
	scheduler_sum_df.to_csv("/home/hong/result_figure/tmp_sum_result.csv")

if __name__ == '__main__':
	df = plotThroughputs('/home/hong/result_figure/statistic.csv')
	plotThroughputSummary(df)
