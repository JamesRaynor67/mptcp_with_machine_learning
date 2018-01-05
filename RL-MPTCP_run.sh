#!/bin/bash
export DISPLAY=:0 # This is only for remote execution
declare -A PyConfig; declare -A Ns3Config

tcpBuffer="$1"
routerBBuffer="$2"
routerCBuffer="$3"
link_b_BER="$4"
topology_id="0"

function runSet() {
  python ./machineLearning/rl_server.py -f "${PyConfig["forceReply"]}" -m "${PyConfig["maxEpisode"]}" &
  sleep 3
  ./waf --run scratch/run/run --command="%s --link_a_BW="${Ns3Config["link_a_BW"]}" --link_b_BW="${Ns3Config["link_b_BW"]}" --link_c_BW="${Ns3Config["link_c_BW"]}"\
                                --link_a_delay="${Ns3Config["link_a_delay"]}" --link_b_delay="${Ns3Config["link_b_delay"]}" --link_c_delay="${Ns3Config["link_c_delay"]}"\
                                --link_b_BER="${Ns3Config["link_b_BER"]}" --tcp_buffer_size="$tcpBuffer" --router_b_buffer_size="$routerBBuffer"\
                                --router_c_buffer_size="$routerCBuffer" --topology_id="$topology_id""
  python ./machineLearning/log_analyzer.py -e "${PyConfig["experiment"]}" -s "${PyConfig["scheduler"]}" -n "${PyConfig["episodeNum"]}" -b "${Ns3Config["link_b_BW"]}" -c "${Ns3Config["link_c_BW"]}" -d "$dirPath"
}

# Parameter setting for groups of experiment.
dirPath="/home/hong/result_figure/0_static_20170609_${tcpBuffer}_${routerBBuffer}_${routerCBuffer}_${link_b_BER}"
cp "/home/hong/result_figure/template.csv" "/home/hong/result_figure/statistic.csv"
mkdir $dirPath

unset PyConfig; unset Ns3Config; declare -A PyConfig; declare -A Ns3Config
scheduler="RR"; experiment="Exp11"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
Ns3Config+=(["link_a_BW"]="200Kbps" ["link_b_BW"]="300Kbps" ["link_c_BW"]="100Kbps" ["link_a_delay"]="6ms" ["link_b_delay"]="15ms" ["link_c_delay"]="15ms" ["link_b_BER"]="0")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="RTT"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="RD"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="L-DBP"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

#####################
unset PyConfig; unset Ns3Config; declare -A PyConfig; declare -A Ns3Config
scheduler="RR"; experiment="Exp12"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
Ns3Config+=(["link_a_BW"]="200Kbps" ["link_b_BW"]="300Kbps" ["link_c_BW"]="100Kbps" ["link_a_delay"]="6ms" ["link_b_delay"]="250ms" ["link_c_delay"]="15ms" ["link_b_BER"]="0")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="RTT"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="RD"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="L-DBP"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

#####################
unset PyConfig; unset Ns3Config; declare -A PyConfig; declare -A Ns3Config
scheduler="RR"; experiment="Exp13"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
Ns3Config+=(["link_a_BW"]="200Kbps" ["link_b_BW"]="300Kbps" ["link_c_BW"]="100Kbps" ["link_a_delay"]="6ms" ["link_b_delay"]="250ms" ["link_c_delay"]="15ms" ["link_b_BER"]="$link_b_BER")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="RTT"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="RD"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="L-DBP"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

#####################
unset PyConfig; unset Ns3Config; declare -A PyConfig; declare -A Ns3Config
scheduler="RR"; experiment="Exp14"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
Ns3Config+=(["link_a_BW"]="400Kbps" ["link_b_BW"]="100Kbps" ["link_c_BW"]="100Kbps" ["link_a_delay"]="6ms" ["link_b_delay"]="15ms" ["link_c_delay"]="15ms" ["link_b_BER"]="0")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="RTT"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="RD"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="L-DBP"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

#####################
unset PyConfig; unset Ns3Config; declare -A PyConfig; declare -A Ns3Config
scheduler="RR"; experiment="Exp15"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
Ns3Config+=(["link_a_BW"]="400Kbps" ["link_b_BW"]="100Kbps" ["link_c_BW"]="100Kbps" ["link_a_delay"]="6ms" ["link_b_delay"]="250ms" ["link_c_delay"]="15ms" ["link_b_BER"]="0")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="RTT"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="RD"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="L-DBP"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

#####################
unset PyConfig; unset Ns3Config; declare -A PyConfig; declare -A Ns3Config
scheduler="RR"; experiment="Exp16"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
Ns3Config+=(["link_a_BW"]="400Kbps" ["link_b_BW"]="100Kbps" ["link_c_BW"]="100Kbps" ["link_a_delay"]="6ms" ["link_b_delay"]="250ms" ["link_c_delay"]="15ms" ["link_b_BER"]="$link_b_BER")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="RTT"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="RD"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="L-DBP"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

#####################
unset PyConfig; unset Ns3Config; declare -A PyConfig; declare -A Ns3Config
scheduler="RR"; experiment="Exp17"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
Ns3Config+=(["link_a_BW"]="400Kbps" ["link_b_BW"]="200Kbps" ["link_c_BW"]="50Kbps" ["link_a_delay"]="6ms" ["link_b_delay"]="15ms" ["link_c_delay"]="15ms" ["link_b_BER"]="0")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="RTT"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="RD"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="L-DBP"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

#####################
unset PyConfig; unset Ns3Config; declare -A PyConfig; declare -A Ns3Config
scheduler="RR"; experiment="Exp18"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
Ns3Config+=(["link_a_BW"]="400Kbps" ["link_b_BW"]="200Kbps" ["link_c_BW"]="50Kbps" ["link_a_delay"]="6ms" ["link_b_delay"]="250ms" ["link_c_delay"]="15ms" ["link_b_BER"]="0")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="RTT"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="RD"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="L-DBP"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

#####################
unset PyConfig; unset Ns3Config; declare -A PyConfig; declare -A Ns3Config
scheduler="RR"; experiment="Exp19"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
Ns3Config+=(["link_a_BW"]="400Kbps" ["link_b_BW"]="200Kbps" ["link_c_BW"]="50Kbps" ["link_a_delay"]="6ms" ["link_b_delay"]="250ms" ["link_c_delay"]="15ms" ["link_b_BER"]="$link_b_BER")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="RTT"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="RD"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="L-DBP"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

#####################
unset PyConfig; unset Ns3Config; declare -A PyConfig; declare -A Ns3Config
scheduler="RR"; experiment="Exp20"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
Ns3Config+=(["link_a_BW"]="400Kbps" ["link_b_BW"]="200Kbps" ["link_c_BW"]="100Kbps" ["link_a_delay"]="6ms" ["link_b_delay"]="250ms" ["link_c_delay"]="15ms" ["link_b_BER"]="$link_b_BER")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="RTT"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="RD"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

unset PyConfig; declare -A PyConfig;
scheduler="L-DBP"
PyConfig+=(["experiment"]=$experiment ["forceReply"]=$scheduler ["maxEpisode"]="1" ["scheduler"]=$scheduler ["episodeNum"]="0")
runSet

# generate statistic of throughput (and others) results
python ./machineLearning/log_bytes.py
cp "/home/hong/result_figure/rcv.png" "${dirPath}/summary/rcv_${tcpBuffer}_${routerBBuffer}_${routerCBuffer}_${link_b_BER}.png"
cp "/home/hong/result_figure/sent.png" "${dirPath}/summary/sent_${tcpBuffer}_${routerBBuffer}_${routerCBuffer}_${link_b_BER}.png"
cp "/home/hong/result_figure/statistic.csv" "${dirPath}/summary/statistic_${tcpBuffer}_${routerBBuffer}_${routerCBuffer}_${link_b_BER}.csv"
mv "/home/hong/result_figure/rcv.png" "${dirPath}/rcv_${tcpBuffer}_${routerBBuffer}_${routerCBuffer}.png"
mv "/home/hong/result_figure/sent.png" "${dirPath}/sent_${tcpBuffer}_${routerBBuffer}_${routerCBuffer}.png"
mv "/home/hong/result_figure/statistic.csv" "${dirPath}/statistic_${tcpBuffer}_${routerBBuffer}_${routerCBuffer}.csv"
