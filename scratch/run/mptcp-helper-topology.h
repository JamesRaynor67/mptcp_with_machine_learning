#pragma once

#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"

namespace ns3{

InternetStackHelper GetInternetStackHelper ();

NetDeviceContainer PointToPointCreate(Ptr<Node> startNode,
                                      Ptr<Node> endNode,
                                      DataRate linkRate,
                                      Time delay,
                                      uint32_t packetSize);

void CreateMultipleFlowsSingleBottleneck (uint32_t interfaceCount,
                                          uint32_t packetSize,
                                          DataRate linkRate,
                                          Time delay,
                                          NodeContainer& servers,
                                          NodeContainer& switches,
                                          NodeContainer& clients,
                                          Ipv4Address& remoteClient);

void CreateMultipleFlowsNoBottleneck (uint32_t interfaceCount,
                                      uint32_t packetSize,
                                      DataRate linkRate,
                                      Time delay,
                                      NodeContainer& servers,
                                      NodeContainer& switches,
                                      NodeContainer& clients,
                                      Ipv4Address& remoteClient);

void CreateMultipleAndTcpFlows (uint32_t interfaceCount,
                                uint32_t packetSize,
                                DataRate linkRate,
                                Time delay,
                                NodeContainer& servers,
                                NodeContainer& switches,
                                NodeContainer& clients,
                                Ipv4Address& remoteClient);

};
