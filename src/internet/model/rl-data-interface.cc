#include <string>
#include "rl-data-interface.h"
#include "rl-socketException.h"

namespace rl{

InterfaceToRL::InterfaceToRL(std::string ip, int port){
  if(!RealSocket::create()){
    throw SocketException("Could not create client socket.");
  }
  if(!RealSocket::connect(ip, port)){
    throw SocketException("Could not bind to port.");
  }
}

const InterfaceToRL& InterfaceToRL::add(const std::string name, const uint32_t val){
  m_send_str.append(name);
  m_send_str.append("#");
  m_send_str.append(std::to_string(val));
  m_send_str.append("$");
  return *this;
}

const InterfaceToRL& InterfaceToRL::add(const std::string name, const double val){
  m_send_str.append(name);
  m_send_str.append("#");
  m_send_str.append(std::to_string(val));
  m_send_str.append("$");
  return *this;
}

void InterfaceToRL::send(){
  if(!RealSocket::send(this->m_send_str)){
    throw SocketException("Could not write to socket.");
  }
  this->m_send_str.clear();
}

std::string InterfaceToRL::recv(){
  if(!RealSocket::recv(this->m_receive_str)){
    throw SocketException ( "Could not read from socket." );
  }
  return this->m_receive_str;
}

std::string InterfaceToRL::get_send_str(){
  return m_send_str;
}

}
