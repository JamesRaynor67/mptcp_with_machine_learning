#include <string>
#include "rl-data-interface.h"
#include "rl-socketException.h"

rl::InterfaceToRL* rl_socket;

namespace rl{

InterfaceToRL::InterfaceToRL(std::string ip, int port):m_added_pair_number(0){
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
  m_added_pair_number++;
  return *this;
}

const InterfaceToRL& InterfaceToRL::add(const std::string name, const uint64_t val){
  m_send_str.append(name);
  m_send_str.append("#");
  m_send_str.append(std::to_string(val));
  m_send_str.append("$");
  m_added_pair_number++;
  return *this;
}

const InterfaceToRL& InterfaceToRL::add(const std::string name, const int64_t val){
  m_send_str.append(name);
  m_send_str.append("#");
  m_send_str.append(std::to_string(val));
  m_send_str.append("$");
  m_added_pair_number++;
  return *this;
}

const InterfaceToRL& InterfaceToRL::add(const std::string name, const double val){
  m_send_str.append(name);
  m_send_str.append("#");
  m_send_str.append(std::to_string(val));
  m_send_str.append("$");
  m_added_pair_number++;
  return *this;
}

const InterfaceToRL& InterfaceToRL::add(const std::string name, const std::string val){
  m_send_str.append(name);
  m_send_str.append("#");
  m_send_str.append(val);
  m_send_str.append("$");
  m_added_pair_number++;
  return *this;
}

void InterfaceToRL::send(){
  // m_added_pair_number plus 1 to include itself
  this->add("size", m_added_pair_number+1);
  m_send_str.append("\r\n");
  if(!RealSocket::send(this->m_send_str)){
    throw SocketException("Could not write to socket.");
  }
  this->m_send_str.clear();
  m_added_pair_number = 0;
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

// static int InterfaceToRL::GetPortOffset(){
//   return m_port_offset;
// }

}
