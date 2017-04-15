#pragma once

#include <string>
#include "rl-realSocket.h"

namespace rl{

class InterfaceToRL : private RealSocket{
public:
  InterfaceToRL(std::string ip, int port);
  virtual ~InterfaceToRL(){};

  // Add name-value pair to send string
  const InterfaceToRL& add(const std::string name, const uint32_t val);
  const InterfaceToRL& add(const std::string name, const uint64_t val);
  const InterfaceToRL& add(const std::string name, const int64_t val);
  const InterfaceToRL& add(const std::string name, const double val);
  const InterfaceToRL& add(const std::string name, const std::string val);

  std::string get_send_str();

  void send();

  std::string recv();

private:
  // This is the string to be sent.
  // Its pattern is "name0$value0#name1$value1#...",
  // where name is a string for name and value is a double value
  std::string m_send_str;

  // This is the string to be sent.
  // Its pattern is "name0$value0#name1$value1#...",
  // where name is a string for name and value is a double value
  std::string m_receive_str;

  // We assume 32 bits are enough, and don't care about overflow, not used yet
  uint32_t m_last_sent_index;

  uint32_t m_added_pair_number;
};

}
