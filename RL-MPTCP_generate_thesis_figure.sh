#!/bin/bash

dirPath="${1}"
factor="${2}"

python /home/hong/workspace/mptcp/ns3/output_analyzer/analyze_scheduler_choose.py $dirPath $factor
python /home/hong/workspace/mptcp/ns3/output_analyzer/analyze_scheduler_total_throughput.py $dirPath $factor
# python /home/hong/workspace/mptcp/ns3/output_analyzer/analyze_scheduler_total_throughput_ns3.py $dirPath $factor
python /home/hong/workspace/mptcp/ns3/output_analyzer/analyze_time_throughput.py $dirPath $factor
python /home/hong/workspace/mptcp/ns3/output_analyzer/analyze_time_txBuffer.py $dirPath $factor
