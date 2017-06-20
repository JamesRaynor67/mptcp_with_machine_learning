#pragma once
#include <utility>
#include <vector>
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"

namespace ns3{

InternetStackHelper GetInternetStackHelper (bool useStaticRouting = false);

NetDeviceContainer PointToPointCreate(Ptr<Node> startNode,
                                      Ptr<Node> endNode,
                                      DataRate linkRate,
                                      Time delay,
                                      uint32_t packetSize,
                                      uint32_t queueSize = 1);

void CreateSimplestNetwork (uint32_t packetSize,
                        NodeContainer& server,
                        NodeContainer& client,
                        NodeContainer& middle,
                        NodeContainer& other_servers,
                        NodeContainer& other_clients,
                        NetDeviceContainer& traceQueueDevices,
                        NetDeviceContainer& unstableDevices);


void CreateNetwork5 (uint32_t packetSize,
                        NodeContainer& server,
                        NodeContainer& client,
                        NodeContainer& middle,
                        NodeContainer& other_servers,
                        NodeContainer& other_clients,
                        NetDeviceContainer& traceQueueDevices,
                        NetDeviceContainer& unstableDevices);

void CreateNetwork11 (uint32_t packetSize,
                        NodeContainer& server,
                        NodeContainer& client,
                        NodeContainer& middle,
                        NodeContainer& other_servers,
                        NodeContainer& other_clients,
                        NetDeviceContainer& traceQueueDevices,
                        NetDeviceContainer& unstableDevices);

void ChangeLinkErrorRate (Ptr<NetDevice> nd, double bitErrorRate);

}
