#pragma once
#include <utility>
#include <vector>
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"

namespace ns3{

InternetStackHelper GetInternetStackHelper ();

NetDeviceContainer PointToPointCreate(Ptr<Node> startNode,
                                      Ptr<Node> endNode,
                                      DataRate linkRate,
                                      Time delay,
                                      uint32_t packetSize);

void CreateRealNetwork (uint32_t packetSize,
                        NodeContainer& server,
                        NodeContainer& client,
                        NodeContainer& isps,
                        NodeContainer& ixs,
                        NodeContainer& other_servers,
                        NodeContainer& other_clients);
                        // std::vector<std::pair<Ipv4Address, Ipv4Address>>& SCIpPairs
};
