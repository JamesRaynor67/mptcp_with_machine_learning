#!/bin/bash
#export QT_QPA_PLATFORM=offscreen
export DISPLAY=:0  # # This is only for remote execution. However, above is recommended on the Internet (but not works perfect
declare -A RLConfig; declare -A Ns3Config

# tcpBuffer="$1"    
# routerBBuffer="$2"
# routerCBuffer="$3"
# link_b_BER="$4"
# topology_id="0"
scheduler="RL-Choose"
dirPath=""

function preProcess(){
  timestamp=$(date +"%Y-%m-%d_%H-%M-%S")
  dirPath="/home/hong/result_figure/0_static_${timestamp}"
  cp "/home/hong/result_figure/template.csv" "/home/hong/result_figure/statistic.csv"
  mkdir "$dirPath"
  cp "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/RL-MPTCP_experiment_parameters.sh" "$dirPath/script_parameters.sh"  
  cp "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/${BASH_SOURCE[0]}" "$dirPath/script_train.sh"
  mkdir "$dirPath/NS3Code"
  mkdir "$dirPath/machineLearning"
  cp -a "/home/hong/workspace/mptcp/ns3/scratch/run/." "$dirPath/NS3Code/"
  cp /home/hong/workspace/mptcp/ns3/machineLearning/*.py "$dirPath/machineLearning"
  source ./RL-MPTCP_experiment_parameters.sh
}

function postProcess(){
  local newName="${dirPath}/statistic_${Ns3Config["tcp_buffer"]}_${Ns3Config["router_b_buffer"]}_${Ns3Config["router_c_buffer"]}_${Ns3Config["link_b_BER"]}_${Ns3Config["topology_id"]}.csv"
  mv "/home/hong/result_figure/statistic.csv" ${newName}
}

function runRL(){
  python ./machineLearning/rl_server.py -m "${RLConfig["maxEpisode"]}" -i "${RLConfig["sendInterval"]}" -p "${RLConfig["savePath"]}" &
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
  RLConfig+=(["forceReply"]=$scheduler ["maxEpisode"]=16 ["scheduler"]=$scheduler ["sendInterval"]="100000" ["savePath"]="${dirPath}")
}

#####################
preProcess
loadRLPara
runRL
sleep 2
for (( episodeNum=0; episodeNum<${RLConfig["maxEpisode"]}; episodeNum++ ))
do
  unset Ns3Config; declare -A Ns3Config
  
  case "$(( ( RANDOM % 16 ) ))" in
  0) loadDefaultBufferSetting; loadParamExp21
    ;;
  1) loadDefaultBufferSetting; loadParamExp22
    ;;    
  2) loadDefaultBufferSetting; loadParamExp23
    ;;
  3) loadDefaultBufferSetting; loadParamExp24
    ;;
  4) loadDefaultBufferSetting; loadParamExp25
    ;;
  5) loadDefaultBufferSetting; loadParamExp26
    ;;
  6) loadDefaultBufferSetting; loadParamExp27
    ;;
  7) loadDefaultBufferSetting; loadParamExp28
    ;;    
  8) loadDefaultBufferSetting; loadParamExp29
    ;;
  9) loadDefaultBufferSetting; loadParamExp30
    ;;
  10) loadDefaultBufferSetting; loadParamExp31
    ;;
  11) loadDefaultBufferSetting; loadParamExp32
    ;;
  12) loadDefaultBufferSetting; loadParamExp33
    ;;    
  13) loadDefaultBufferSetting; loadParamExp34
    ;;
  14) loadDefaultBufferSetting; loadParamExp35
    ;;
  15) loadDefaultBufferSetting; loadParamExp36
    ;;
  *) echo 'Error!'; exit
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
modelFile="$(find ${dirPath} -name my_final_model*meta)"
bash ./RL-MPTCP_test.sh "${modelFile}"
