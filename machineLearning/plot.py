"""
========
Barchart
========

A bar plot with errorbars and height labels on individual bars
"""
import pandas as pd
import numpy as np
import seaborn as sns

df = pd.DataFrame({'Sent Bytes': [2749504, 2749504, 2749504, 2749504, 1371208, 1382552,
								4156160, 4027122, 4158996, 4082424, 2796298,1381134, 
								3630082, 3739268, 3669786, 3525150, 2796298, 1145746,
								2823240, 2823240, 2823240, 2820404, 2796298, 35452,
								4171758, 4316394, 4158996, 4221388, 4280944, 1381134,
								4275272, 4280944, 4275272, 4290870, 4280944, 35870],
				   'Received Bytes': [2721144, 2721144, 2721144, 2721144, 1358446, 1366954,
				   					4124964, 3994508, 4126382, 4049810, 2780700, 1365536,
				   					3605976, 3712326, 3644262, 3503880, 2780700, 1140074,
				   					2804806, 2804806, 2804806, 2803388, 2780700, 34030,
				   					4122128, 4262510, 4107948, 4168922, 4262510, 1366954,
				   					4256838, 4258256, 4256838, 4259674, 4262510, 35452],
				   'Scheduler': ['Round-Robin', 'Fastest-RTT', 'Random', 'Largest-DBP', 'Only Subflow 0', 'Only Subflow 1',
  				   				'Round-Robin', 'Fastest-RTT', 'Random', 'Largest-DBP', 'Only Subflow 0', 'Only Subflow 1',
  				   				'Round-Robin', 'Fastest-RTT', 'Random', 'Largest-DBP', 'Only Subflow 0', 'Only Subflow 1',
  				   				'Round-Robin', 'Fastest-RTT', 'Random', 'Largest-DBP', 'Only Subflow 0', 'Only Subflow 1',
  				   				'Round-Robin', 'Fastest-RTT', 'Random', 'Largest-DBP', 'Only Subflow 0', 'Only Subflow 1',
				   				'Round-Robin', 'Fastest-RTT', 'Random', 'Largest-DBP', 'Only Subflow 0', 'Only Subflow 1'],
				   'Experiment': ['Exp 1', 'Exp 1', 'Exp 1', 'Exp 1', 'Exp 1', 'Exp 1',
				   				'Exp 2', 'Exp 2', 'Exp 2', 'Exp 2', 'Exp 2', 'Exp 2',
				   				'Exp 3', 'Exp 3', 'Exp 3', 'Exp 3', 'Exp 3', 'Exp 3',
				   				'Exp 4', 'Exp 4', 'Exp 4', 'Exp 4', 'Exp 4', 'Exp 4',
				   				'Exp 5', 'Exp 5', 'Exp 5', 'Exp 5', 'Exp 5', 'Exp 5',
				   				'Exp 6', 'Exp 6', 'Exp 6', 'Exp 6', 'Exp 6', 'Exp 6']})

# sns.set(font_scale=2)
# g = sns.factorplot(x="Experiment", y="Sent Bytes", col="Scheduler", kind="bar", data=df, palette="Set2")
# g.set_axis_labels("Experiment Number", "Sent Bytes").set_xticklabels(['1', '2', '3', '4', '5', '6']).set_titles("{col_name}").despine(left=True)

# sns.set(font_scale=2)
# g = sns.factorplot(x="Scheduler", y="Sent Bytes", col="Experiment", kind="bar", data=df, palette="Set2")
# g.set_axis_labels("Scheduler", "Sent Bytes").set_xticklabels(['1', '2', '3', '4', '5', '6']).set_titles("{col_name}").despine(left=True)
# g.fig.text(0.33, 0.16,
# 		'                                             Mapping from number to scheduler\n'
# 		'1 -- Round-Robin                           '
# 		'2 -- Fastest-RTT                           '
# 		'3 -- Random\n'
# 		'4 -- Largest-DBP                            '
# 		'5 -- Only subflow 0                       '
# 		'6 -- Only subflow 1', fontsize=20)

# sns.set(font_scale=2)
# g = sns.factorplot(x="Experiment", y="Received Bytes", col="Scheduler", kind="bar", data=df, palette="Set2")
# g.set_axis_labels("Experiment Number", "Received Bytes").set_xticklabels(['1', '2', '3', '4', '5', '6']).set_titles("{col_name}").despine(left=True)

sns.set(font_scale=2)
g = sns.factorplot(x="Scheduler", y="Received Bytes", col="Experiment", kind="bar", data=df, palette="Set2")
g.set_axis_labels("Scheduler", "Received Bytes").set_xticklabels(['1', '2', '3', '4', '5', '6']).set_titles("{col_name}").despine(left=True)
g.fig.text(0.33, 0.16,
		'                                             Mapping from number to scheduler\n'
		'1 -- Round-Robin                           '
		'2 -- Fastest-RTT                           '
		'3 -- Random\n'
		'4 -- Largest-DBP                            '
		'5 -- Only subflow 0                       '
		'6 -- Only subflow 1', fontsize=20)

sns.plt.show()

# g.fig.suptitle('this is the figure title', verticalalignment='top', fontsize=20)


