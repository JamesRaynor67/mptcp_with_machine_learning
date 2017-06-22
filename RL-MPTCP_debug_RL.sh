#!/bin/bash
#export QT_QPA_PLATFORM=offscreen
export DISPLAY=:0  # # This is only for remote execution. However, above is recommended on the Internet (but not works perfect
declare -A RLConfig; declare -A Ns3Config

# tcp_buffer="262144"    
# router_b_buffer="100"
# router_c_buffer="100"
# link_b_BER="0"
# topology_id="0"

scheduler="RL-Choose"
dirPath=""

function preProcess(){
  timestamp=$(date +"%Y-%m-%d_%H-%M-%S")
  dirPath="/home/hong/result_figure/0_static_${timestamp}"
  cp "/home/hong/result_figure/template.csv" "/home/hong/result_figure/statistic.csv"
  mkdir $dirPath
  cp "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/${BASH_SOURCE[0]}" "$dirPath/script.sh"
  source ./RL-MPTCP_experiment_parameters.sh
}

function postProcess(){
  local newName="${dirPath}/statistic_${Ns3Config["tcpBuffer"]}_${Ns3Config["linkBBuffer"]}_${Ns3Config["linkCBuffer"]}_${Ns3Config["link_b_BER"]}_${Ns3Config["topology_id"]}.csv"
  mv "/home/hong/result_figure/statistic.csv" ${newName}
}

function runRL(){
  python ./machineLearning/rl_server.py -m "${RLConfig["maxEpisode"]}" -i "${RLConfig["sendInterval"]}" -p "${RLConfig["savePath"]}" &
}

function runNS3(){
#  local episodeNum="${1}"
  ./waf --run scratch/run/run --command="%s --link_a_BW="${Ns3Config["link_a_BW"]}" --link_b_BW="${Ns3Config["link_b_BW"]}" --link_c_BW="${Ns3Config["link_c_BW"]}"\
                              --link_a_delay="${Ns3Config["link_a_delay"]}" --link_b_delay="${Ns3Config["link_b_delay"]}" --link_c_delay="${Ns3Config["link_c_delay"]}"\
                              --link_b_BER="${Ns3Config["link_b_BER"]}" --tcp_buffer_size="${Ns3Config["tcpBuffer"]}" --router_b_buffer_size="${Ns3Config["linkBBuffer"]}"\
                              --router_c_buffer_size="${Ns3Config["linkCBuffer"]}" --topology_id="${Ns3Config["topology_id"]}""
}

function record(){
  local episodeNum="${1}"
  python ./machineLearning/log_analyzer.py -e "${Ns3Config["experiment"]}" -s "${RLConfig["scheduler"]}" -n "$episodeNum" -b "${Ns3Config["link_b_BW"]}" -c "${Ns3Config["link_c_BW"]}" -d "$dirPath"
}

function loadRLPara(){
  RLConfig+=(["forceReply"]=$scheduler ["maxEpisode"]=20 ["scheduler"]=$scheduler ["sendInterval"]="100000" ["savePath"]="${dirPath}")
}

#####################
preProcess
loadRLPara
runRL
sleep 2
for (( episodeNum=0; episodeNum<${RLConfig["maxEpisode"]}; episodeNum++ ))
do
  unset Ns3Config; declare -A Ns3Config
  
  case "$(( ( RANDOM % 2 ) ))" in
  1) loadDefaultBufferSetting; loadParamExp15
    ;;
  *) loadDefaultBufferSetting; loadParamExp17
    ;;
  esac

  runNS3 "$episodeNum"
  # record "$episodeNum"
 if !(($episodeNum % 20))
 then
   record "$episodeNum"
 else
   sleep 3
 fi
done
postProcess

### above is training, below is testing
modelFile="$(find ${dirPath} -name 'events*')"
bash ./RL-MPTCP_test.sh "${modelFile}"
