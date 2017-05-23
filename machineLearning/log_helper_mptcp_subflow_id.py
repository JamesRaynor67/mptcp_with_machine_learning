class MpTcpSubflows(object):
	
	subflowIds = [-1] * 4 # subflowLen == 4

	# global variables
	simulationTime = 120 

	@staticmethod
	def updateSubflowId(From, To, flowId):
		if flowId not in MpTcpSubflows.subflowIds:
			if From == '192.168.9.2' and To == '192.168.0.1':
			    MpTcpSubflows.subflowIds[0] = flowId
			elif From == '192.168.0.1' and To == '192.168.9.2':
			    MpTcpSubflows.subflowIds[1] = flowId
			elif From == '192.168.11.2' and To == '192.168.0.1':
			    MpTcpSubflows.subflowIds[2] = flowId
			elif From == '192.168.0.1' and To == '192.168.11.2':
			    MpTcpSubflows.subflowIds[3] = flowId

	@staticmethod
	def getSubflowId(flowId):
		try:
			return MpTcpSubflows.subflowIds.index(flowId)
		except ValueError:
			return -1

	@staticmethod
	def getSubflowList():
		return MpTcpSubflows.subflowIds
