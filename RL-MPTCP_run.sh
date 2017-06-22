#!/bin/bash
#export QT_QPA_PLATFORM=offscreen
export DISPLAY=:0 # This is only for remote execution
declare -A PyConfig; declare -A Ns3Config

# 8M = 8388608
# 8M = 252144

tcp_buffer="$1"    
router_b_buffer="$2"
router_c_buffer="$3"
link_b_BER="$4"
topology_id="0"

function runSet() {
  python ./machineLearning/rl_server.py -f "${PyConfig["forceReply"]}" -m "${PyConfig["maxEpisode"]}" &
  sleep 3
  ./waf --run scratch/run/run --command="%s --link_a_BW="${Ns3Config["link_a_BW"]}" --link_b_BW="${Ns3Config["link_b_BW"]}" --link_c_BW="${Ns3Config["link_c_BW"]}"\
                                --link_a_delay="${Ns3Config["link_a_delay"]}" --link_b_delay="${Ns3Config["link_b_delay"]}" --link_c_delay="${Ns3Config["link_c_delay"]}"\
                                --link_b_BER="${Ns3Config["link_b_BER"]}" --tcp_buffer_size="${Ns3Config["tcp_buffer"]}" --router_b_buffer_size="${Ns3Config["router_b_buffer"]}"\
                                --router_c_buffer_size="${Ns3Config["router_c_buffer"]}" --topology_id="${Ns3Config["topology_id"]}""
#   > ~/result_figure/0_static_20170604/log_debug_mptcp.txt 2>&1
  python ./machineLearning/log_analyzer.py -e "${Ns3Config["experiment"]}" -s "${PyConfig["scheduler"]}" -n "${PyConfig["episodeNum"]}" -b "${Ns3Config["link_b_BW"]}" -c "${Ns3Config["link_c_BW"]}" -d "$dirPath"
}

function runByAllSchedulers(){
  unset PyConfig; declare -A PyConfig;
  scheduler="RR";
  PyConfig+=(["experiment"]=${Ns3Config["experiment"]} ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
  runSet

  unset PyConfig; declare -A PyConfig;
  scheduler="RTT"
  PyConfig+=(["experiment"]=${Ns3Config["experiment"]} ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
  runSet

  unset PyConfig; declare -A PyConfig;
  scheduler="RD"
  PyConfig+=(["experiment"]=${Ns3Config["experiment"]} ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
  runSet

  unset PyConfig; declare -A PyConfig;
  scheduler="L-DBP"
  PyConfig+=(["experiment"]=${Ns3Config["experiment"]} ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
  runSet
}

#####################
source ./RL-MPTCP_experiment_parameters.sh
timestamp=$(date +"%Y-%m-%d_%H-%M-%S")
dirPath="/home/hong/result_figure/0_static_${timestamp}_${tcp_buffer}_${router_b_buffer}_${router_c_buffer}_${link_b_BER}"
cp "/home/hong/result_figure/template.csv" "/home/hong/result_figure/statistic.csv"
mkdir $dirPath

unset Ns3Config; declare -A Ns3Config
loadParamExp22
runByAllSchedulers

unset Ns3Config; declare -A Ns3Config
loadParamExp23
runByAllSchedulers

python ./machineLearning/log_bytes.py
cp "/home/hong/result_figure/rcv.png" "/home/hong/result_figure/summary/rcv_${timestamp}_${tcp_buffer}_${router_b_buffer}_${router_c_buffer}_${link_b_BER}.png"
cp "/home/hong/result_figure/sent.png" "/home/hong/result_figure/summary/sent_${timestamp}_${tcp_buffer}_${router_b_buffer}_${router_c_buffer}_${link_b_BER}.png"
cp "/home/hong/result_figure/statistic.csv" "/home/hong/result_figure/summary/statistic_${timestamp}_${tcp_buffer}_${router_b_buffer}_${router_c_buffer}_${link_b_BER}.csv"
mv "/home/hong/result_figure/rcv.png" "${dirPath}/rcv_${tcp_buffer}_${router_b_buffer}_${router_c_buffer}.png"
mv "/home/hong/result_figure/sent.png" "${dirPath}/sent_${tcp_buffer}_${router_b_buffer}_${router_c_buffer}.png"
mv "/home/hong/result_figure/statistic.csv" "${dirPath}/statistic_${tcp_buffer}_${router_b_buffer}_${router_c_buffer}.csv"