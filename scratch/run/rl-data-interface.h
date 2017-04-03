#pragma once

#include <string>
#include "rl-realSocket.h"

namespace rl{

class InterfaceToRL : private RealSocket{
public:
  InterfaceToRL(std::string ip, int port);
  virtual ~InterfaceToRL(){};

  // Add name-value pair to send string
  void add(std::string name, double val);

  std::string get_send_str();

  const InterfaceToRL& operator << ( const std::string& ) const;

  const InterfaceToRL& operator >> ( std::string& ) const;

private:
  // This is the string to be sent.
  // Its pattern is "name0$value0#name1$value1#...",
  // where name is a string for name and value is a double value
  std::string m_send_str;

  // This is the string to be sent.
  // Its pattern is "name0$value0#name1$value1#...",
  // where name is a string for name and value is a double value
  std::string m_receive_str;

  // We assume 32 bits are enough, and don't care about overflow
  uint32_t m_last_sent_index;
};


}
