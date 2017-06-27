#!/bin/bash
#export QT_QPA_PLATFORM=offscreen
export DISPLAY=:0 # This is only for remote execution
declare -A PyConfig; declare -A Ns3Config

function checkNoneEmpty(){
  if [ -z "$tcp_buffer" ] || [ -z "$router_b_buffer" ] || [ -z "$router_c_buffer" ] || [ -z "$link_b_BER" ] || [ -z "$topology_id" ];then
    echo "$(tput setaf 1)Empty value! Exit...$(tput sgr 0)"
    exit
  fi
}

function loadDefaultBufferSetting(){
  tcp_buffer="262144";router_b_buffer="100";router_c_buffer="100";link_b_BER="0";topology_id="5"
  echo 'tcp_buffer='$tcp_buffer ' router_b_buffer='$router_b_buffer ' router_c_buffer='$router_b_buffer ' link_b_BER='$link_b_BER ' topology_id='$topology_id 
}

function loadParamExp21() {
  checkNoneEmpty
  Ns3Config+=(["link_a_BW"]="400Kbps" ["link_b_BW"]="100Kbps" ["link_c_BW"]="100Kbps" ["link_a_delay"]="6ms" ["link_b_delay"]="250ms" ["link_c_delay"]="15ms" \
              ["tcp_buffer"]="$tcp_buffer" ["router_b_buffer"]="$router_b_buffer" ["router_c_buffer"]="$router_c_buffer" ["link_b_BER"]="$link_b_BER" ["topology_id"]="$topology_id" ["experiment"]="Exp21")
}

function loadParamExp22() {
  checkNoneEmpty
  Ns3Config+=(["link_a_BW"]="400Kbps" ["link_b_BW"]="100Kbps" ["link_c_BW"]="100Kbps" ["link_a_delay"]="6ms" ["link_b_delay"]="15ms" ["link_c_delay"]="15ms" \
              ["tcp_buffer"]="$tcp_buffer" ["router_b_buffer"]="$router_b_buffer" ["router_c_buffer"]="$router_c_buffer" ["link_b_BER"]="$link_b_BER" ["topology_id"]="$topology_id" ["experiment"]="Exp22")
}

function loadParamExp23() {
  checkNoneEmpty
  Ns3Config+=(["link_a_BW"]="400Kbps" ["link_b_BW"]="200Kbps" ["link_c_BW"]="100Kbps" ["link_a_delay"]="6ms" ["link_b_delay"]="15ms" ["link_c_delay"]="15ms" \
              ["tcp_buffer"]="$tcp_buffer" ["router_b_buffer"]="$router_b_buffer" ["router_c_buffer"]="$router_c_buffer" ["link_b_BER"]="$link_b_BER" ["topology_id"]="$topology_id" ["experiment"]="Exp23")
}

function loadParamExp24() {
  checkNoneEmpty
  Ns3Config+=(["link_a_BW"]="400Kbps" ["link_b_BW"]="200Kbps" ["link_c_BW"]="50Kbps" ["link_a_delay"]="6ms" ["link_b_delay"]="15ms" ["link_c_delay"]="15ms" \
              ["tcp_buffer"]="$tcp_buffer" ["router_b_buffer"]="$router_b_buffer" ["router_c_buffer"]="$router_c_buffer" ["link_b_BER"]="$link_b_BER" ["topology_id"]="$topology_id" ["experiment"]="Exp24")
}

function loadParamExp25() {
  checkNoneEmpty
  Ns3Config+=(["link_a_BW"]="400Kbps" ["link_b_BW"]="200Kbps" ["link_c_BW"]="50Kbps" ["link_a_delay"]="6ms" ["link_b_delay"]="250ms" ["link_c_delay"]="15ms" \
              ["tcp_buffer"]="$tcp_buffer" ["router_b_buffer"]="$router_b_buffer" ["router_c_buffer"]="$router_c_buffer" ["link_b_BER"]="$link_b_BER" ["topology_id"]="$topology_id" ["experiment"]="Exp25")
}

function loadParamExp26() {
  checkNoneEmpty
  Ns3Config+=(["link_a_BW"]="250Kbps" ["link_b_BW"]="100Kbps" ["link_c_BW"]="100Kbps" ["link_a_delay"]="6ms" ["link_b_delay"]="250ms" ["link_c_delay"]="15ms" \
              ["tcp_buffer"]="$tcp_buffer" ["router_b_buffer"]="$router_b_buffer" ["router_c_buffer"]="$router_c_buffer" ["link_b_BER"]="$link_b_BER" ["topology_id"]="$topology_id" ["experiment"]="Exp26")
}

function loadParamExp27() {
  checkNoneEmpty
  Ns3Config+=(["link_a_BW"]="250Kbps" ["link_b_BW"]="100Kbps" ["link_c_BW"]="100Kbps" ["link_a_delay"]="6ms" ["link_b_delay"]="15ms" ["link_c_delay"]="15ms" \
              ["tcp_buffer"]="$tcp_buffer" ["router_b_buffer"]="$router_b_buffer" ["router_c_buffer"]="$router_c_buffer" ["link_b_BER"]="$link_b_BER" ["topology_id"]="$topology_id" ["experiment"]="Exp27")
}

function loadParamExp28() {
  checkNoneEmpty
  Ns3Config+=(["link_a_BW"]="250Kbps" ["link_b_BW"]="200Kbps" ["link_c_BW"]="100Kbps" ["link_a_delay"]="6ms" ["link_b_delay"]="15ms" ["link_c_delay"]="15ms" \
              ["tcp_buffer"]="$tcp_buffer" ["router_b_buffer"]="$router_b_buffer" ["router_c_buffer"]="$router_c_buffer" ["link_b_BER"]="$link_b_BER" ["topology_id"]="$topology_id" ["experiment"]="Exp28")
}

function loadParamExp29() {
  checkNoneEmpty
  Ns3Config+=(["link_a_BW"]="250Kbps" ["link_b_BW"]="200Kbps" ["link_c_BW"]="50Kbps" ["link_a_delay"]="6ms" ["link_b_delay"]="15ms" ["link_c_delay"]="15ms" \
              ["tcp_buffer"]="$tcp_buffer" ["router_b_buffer"]="$router_b_buffer" ["router_c_buffer"]="$router_c_buffer" ["link_b_BER"]="$link_b_BER" ["topology_id"]="$topology_id" ["experiment"]="Exp29")
}

function loadParamExp30() {
  checkNoneEmpty
  Ns3Config+=(["link_a_BW"]="250Kbps" ["link_b_BW"]="200Kbps" ["link_c_BW"]="50Kbps" ["link_a_delay"]="6ms" ["link_b_delay"]="250ms" ["link_c_delay"]="15ms" \
              ["tcp_buffer"]="$tcp_buffer" ["router_b_buffer"]="$router_b_buffer" ["router_c_buffer"]="$router_c_buffer" ["link_b_BER"]="$link_b_BER" ["topology_id"]="$topology_id" ["experiment"]="Exp30")
}

function loadParamExp31() {
  checkNoneEmpty
  Ns3Config+=(["link_a_BW"]="250Kbps" ["link_b_BW"]="150Kbps" ["link_c_BW"]="150Kbps" ["link_a_delay"]="6ms" ["link_b_delay"]="15ms" ["link_c_delay"]="15ms" \
              ["tcp_buffer"]="$tcp_buffer" ["router_b_buffer"]="$router_b_buffer" ["router_c_buffer"]="$router_c_buffer" ["link_b_BER"]="$link_b_BER" ["topology_id"]="$topology_id" ["experiment"]="Exp31")
}

function loadParamExp32() {
  checkNoneEmpty
  Ns3Config+=(["link_a_BW"]="250Kbps" ["link_b_BW"]="200Kbps" ["link_c_BW"]="100Kbps" ["link_a_delay"]="6ms" ["link_b_delay"]="15ms" ["link_c_delay"]="15ms" \
              ["tcp_buffer"]="$tcp_buffer" ["router_b_buffer"]="$router_b_buffer" ["router_c_buffer"]="$router_c_buffer" ["link_b_BER"]="$link_b_BER" ["topology_id"]="$topology_id" ["experiment"]="Exp32")
}

function loadParamExp33() {
  checkNoneEmpty
  Ns3Config+=(["link_a_BW"]="250Kbps" ["link_b_BW"]="250Kbps" ["link_c_BW"]="50Kbps" ["link_a_delay"]="6ms" ["link_b_delay"]="15ms" ["link_c_delay"]="15ms" \
              ["tcp_buffer"]="$tcp_buffer" ["router_b_buffer"]="$router_b_buffer" ["router_c_buffer"]="$router_c_buffer" ["link_b_BER"]="$link_b_BER" ["topology_id"]="$topology_id" ["experiment"]="Exp33")
}

function loadParamExp34() {
  checkNoneEmpty
  Ns3Config+=(["link_a_BW"]="400Kbps" ["link_b_BW"]="150Kbps" ["link_c_BW"]="150Kbps" ["link_a_delay"]="6ms" ["link_b_delay"]="15ms" ["link_c_delay"]="15ms" \
              ["tcp_buffer"]="$tcp_buffer" ["router_b_buffer"]="$router_b_buffer" ["router_c_buffer"]="$router_c_buffer" ["link_b_BER"]="$link_b_BER" ["topology_id"]="$topology_id" ["experiment"]="Exp34")
}

function loadParamExp35() {
  checkNoneEmpty
  Ns3Config+=(["link_a_BW"]="400Kbps" ["link_b_BW"]="200Kbps" ["link_c_BW"]="100Kbps" ["link_a_delay"]="6ms" ["link_b_delay"]="15ms" ["link_c_delay"]="15ms" \
              ["tcp_buffer"]="$tcp_buffer" ["router_b_buffer"]="$router_b_buffer" ["router_c_buffer"]="$router_c_buffer" ["link_b_BER"]="$link_b_BER" ["topology_id"]="$topology_id" ["experiment"]="Exp35")
}

function loadParamExp36() {
  checkNoneEmpty
  Ns3Config+=(["link_a_BW"]="400Kbps" ["link_b_BW"]="250Kbps" ["link_c_BW"]="50Kbps" ["link_a_delay"]="6ms" ["link_b_delay"]="15ms" ["link_c_delay"]="15ms" \
              ["tcp_buffer"]="$tcp_buffer" ["router_b_buffer"]="$router_b_buffer" ["router_c_buffer"]="$router_c_buffer" ["link_b_BER"]="$link_b_BER" ["topology_id"]="$topology_id" ["experiment"]="Exp36")
}







function loadParamExp999() {
  checkNoneEmpty
  Ns3Config+=(["link_a_BW"]="50Kbps" ["link_b_BW"]="200Kbps" ["link_c_BW"]="100Kbps" ["link_a_delay"]="6ms" ["link_b_delay"]="15ms" ["link_c_delay"]="15ms" \
              ["tcp_buffer"]="$tcp_buffer" ["router_b_buffer"]="$router_b_buffer" ["router_c_buffer"]="$router_c_buffer" ["link_b_BER"]="$link_b_BER" ["topology_id"]="$topology_id" ["experiment"]="Exp999")
}
