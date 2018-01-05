#!/bin/bash

#export QT_QPA_PLATFORM=offscreen
export DISPLAY=:0 # This is only for remote execution. However, above is recommended on the Internet (but not works perfect
declare -A PyConfig; declare -A Ns3Config

tcpBuffer="252144"
routerBBuffer="1"
routerCBuffer="500"
link_b_BER="0.000002"
topology_id="0"

function runSet() {
  dirPath="/home/hong/result_figure/0_static_20170607_${tcpBuffer}_${routerBBuffer}_${routerCBuffer}_${link_b_BER}"
  cp "/home/hong/result_figure/template.csv" "/home/hong/result_figure/statistic.csv"
  mkdir $dirPath
  python ./machineLearning/rl_server.py -f "${PyConfig["forceReply"]}" -m "${PyConfig["maxEpisode"]}" &
  # Delay to ensure rl_server.py starts to listen port before the start of ns3. (if not, rl::SocketException is thrown)
  sleep 3
  ./waf --run scratch/run/run --command="%s --link_a_BW="${Ns3Config["link_a_BW"]}" --link_b_BW="${Ns3Config["link_b_BW"]}" --link_c_BW="${Ns3Config["link_c_BW"]}"\
                                --link_a_delay="${Ns3Config["link_a_delay"]}" --link_b_delay="${Ns3Config["link_b_delay"]}" --link_c_delay="${Ns3Config["link_c_delay"]}"\
                                --link_b_BER="${Ns3Config["link_b_BER"]}" --tcp_buffer_size="$tcpBuffer" --router_b_buffer_size="$routerBBuffer"\
                                --router_c_buffer_size="$routerCBuffer" --topology_id="$topology_id""

  python ./machineLearning/log_analyzer.py -e "${PyConfig["experiment"]}" -s "${PyConfig["scheduler"]}" -n "${PyConfig["episodeNum"]}" -b "${Ns3Config["link_b_BW"]}" -c "${Ns3Config["link_c_BW"]}" -d "$dirPath"
  mv "/home/hong/result_figure/statistic.csv" "${dirPath}/statistic.csv"
}

#####################
unset PyConfig; unset Ns3Config; declare -A PyConfig; declare -A Ns3Config
scheduler="RTT"; experiment="Exp11"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
Ns3Config+=(["link_a_BW"]="400Kbps" ["link_b_BW"]="200Kbps" ["link_c_BW"]="100Kbps" ["link_a_delay"]="6ms" ["link_b_delay"]="15ms" ["link_c_delay"]="15ms" ["link_b_BER"]="$link_b_BER")
runSet
