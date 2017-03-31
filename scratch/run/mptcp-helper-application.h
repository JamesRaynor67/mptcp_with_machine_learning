#pragma once

#include "ns3/internet-module.h"

namespace ns3{

Ptr<Application> CreateApplication (Address& remoteAddress,
                                    DataRate dataRate,
                                    uint32_t packetSize);

void InstallOnOffApplications(NodeContainer& servers,
                              NodeContainer& clients,
                              const Ipv4Address& peer,
                              uint32_t packetSize);

void InstallFileTransferApplications(NodeContainer& servers,
                                     NodeContainer& clients,
                                     const Ipv4Address& peer,
                                     uint32_t packetSize,
                                     uint32_t queueSize);

};
