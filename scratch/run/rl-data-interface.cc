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

inline void InterfaceToRL::add(std::string name, double val){
  m_send_str.append(name);
  m_send_str.append("$");
  m_send_str.append(std::to_string(val));
  m_send_str.append("#");
}

inline std::string InterfaceToRL::get_send_str(){
  return m_send_str;
}

const InterfaceToRL& InterfaceToRL::operator << ( const std::string& s ) const{
  if(!RealSocket::send(s)){
    throw SocketException("Could not write to socket.");
  }
  return *this;
}


const InterfaceToRL& InterfaceToRL::operator >> ( std::string& s ) const{
  if(!RealSocket::recv(s)){
    throw SocketException ( "Could not read from socket." );
  }
  return *this;
}

}
