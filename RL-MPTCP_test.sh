#!/bin/bash
#export QT_QPA_PLATFORM=offscreen
export DISPLAY=:0  # # This is only for remote execution. However, above is recommended on the Internet (but not works perfect
declare -A RLConfig; declare -A Ns3Config

scheduler="RL-Choose"
dirPath=""
maxEpisode=6
# restoreFromFile="/home/hong/result_figure/0_static_20170612_2017-06-12_17-21-18/my_model-169977.meta"
restoreFromFile="${1}" # This is the input of this script

function preProcess(){
  timestamp=$(date +"%Y-%m-%d_%H-%M-%S")
  dirPath="/home/hong/result_figure/0_static_${timestamp}_test"
  cp "/home/hong/result_figure/template.csv" "/home/hong/result_figure/statistic.csv"
  mkdir $dirPath
  cp "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/${BASH_SOURCE[0]}" "$dirPath/script_test.sh"
  cp "$restoreFromFile" "$dirPath/$(basename $restoreFromFile)"
  source ./RL-MPTCP_experiment_parameters.sh
}

function postProcess(){
  local newName="${dirPath}/statistic_${Ns3Config["tcpBuffer"]}_${Ns3Config["linkBBuffer"]}_${Ns3Config["linkCBuffer"]}_${Ns3Config["link_b_BER"]}_${Ns3Config["topology_id"]}.csv"
  mv "/home/hong/result_figure/statistic.csv" ${newName}
}

function runRL(){
  echo "${restoreFromFile}"
  python ./machineLearning/rl_test.py -m "${RLConfig["maxEpisode"]}" -i "${RLConfig["sendInterval"]}" -p "${RLConfig["savePath"]}" -r "${restoreFromFile}"&
}

function runNS3(){
  ./waf --run scratch/run/run --command="%s --link_a_BW="${Ns3Config["link_a_BW"]}" --link_b_BW="${Ns3Config["link_b_BW"]}" --link_c_BW="${Ns3Config["link_c_BW"]}"\
                                --link_a_delay="${Ns3Config["link_a_delay"]}" --link_b_delay="${Ns3Config["link_b_delay"]}" --link_c_delay="${Ns3Config["link_c_delay"]}"\
                                --link_b_BER="${Ns3Config["link_b_BER"]}" --tcp_buffer_size="${Ns3Config["tcp_buffer"]}" --router_b_buffer_size="${Ns3Config["router_b_buffer"]}"\
                                --router_c_buffer_size="${Ns3Config["router_c_buffer"]}" --topology_id="${Ns3Config["topology_id"]}""
}

function record(){
  local episodeNum="${1}"
  python ./machineLearning/log_analyzer.py -e "${Ns3Config["experiment"]}" -s "${RLConfig["scheduler"]}" -n "$episodeNum" -b "${Ns3Config["link_b_BW"]}" -c "${Ns3Config["link_c_BW"]}" -d "$dirPath"
}

function loadRLPara(){
  RLConfig+=(["forceReply"]=$scheduler ["maxEpisode"]=$maxEpisode ["scheduler"]=$scheduler ["sendInterval"]="100000" ["savePath"]="${dirPath}")
}

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

######## Below to do testing by trained model (restoreFromFile) #######
preProcess
loadRLPara
runRL
sleep 8  # restore takes some longer time, may need to set this value larger in the future
for (( episodeNum=0; episodeNum<${RLConfig["maxEpisode"]}; episodeNum++ ))
do
  unset Ns3Config; declare -A Ns3Config
  
  case "$(( ( $episodeNum % 6 ) ))" in
  0) loadDefaultBufferSetting; loadParamExp31
    ;;
  1) loadDefaultBufferSetting; loadParamExp32
    ;;    
  2) loadDefaultBufferSetting; loadParamExp33
    ;;
  3) loadDefaultBufferSetting; loadParamExp34
    ;;
  4) loadDefaultBufferSetting; loadParamExp35
    ;;
  5) loadDefaultBufferSetting; loadParamExp36
    ;;
  *) echo 'Error!'; exit
    ;;
  esac
  
  runNS3
  record "$episodeNum"

done


######## Below apply 4 schedulers to each case to compare #######
for (( episodeNum=0; episodeNum<6; episodeNum++ ))
do
  unset Ns3Config; declare -A Ns3Config
  
  case "$(( ( $episodeNum % 6 ) ))" in
  0) loadDefaultBufferSetting; loadParamExp31
    ;;
  1) loadDefaultBufferSetting; loadParamExp32
    ;;    
  2) loadDefaultBufferSetting; loadParamExp33
    ;;
  3) loadDefaultBufferSetting; loadParamExp34
    ;;
  4) loadDefaultBufferSetting; loadParamExp35
    ;;
  5) loadDefaultBufferSetting; loadParamExp36
    ;;
  *) echo 'Error!'; exit
    ;;
  esac
  runByAllSchedulers
done

python ./machineLearning/log_bytes.py
cp "/home/hong/result_figure/rcv.png" "/home/hong/result_figure/summary/rcv_${timestamp}_${tcp_buffer}_${router_b_buffer}_${router_c_buffer}_${link_b_BER}.png"
cp "/home/hong/result_figure/sent.png" "/home/hong/result_figure/summary/sent_${timestamp}_${tcp_buffer}_${router_b_buffer}_${router_c_buffer}_${link_b_BER}.png"
cp "/home/hong/result_figure/statistic.csv" "/home/hong/result_figure/summary/statistic_${timestamp}_${tcp_buffer}_${router_b_buffer}_${router_c_buffer}_${link_b_BER}.csv"
mv "/home/hong/result_figure/rcv.png" "${dirPath}/rcv_${tcp_buffer}_${router_b_buffer}_${router_c_buffer}.png"
mv "/home/hong/result_figure/sent.png" "${dirPath}/sent_${tcp_buffer}_${router_b_buffer}_${router_c_buffer}.png"
mv "/home/hong/result_figure/statistic.csv" "${dirPath}/statistic_${tcp_buffer}_${router_b_buffer}_${router_c_buffer}.csv"
