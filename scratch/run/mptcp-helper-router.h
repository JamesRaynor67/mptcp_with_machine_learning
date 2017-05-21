#pragma once

#include "ns3/internet-module.h"

namespace ns3{

void PrintRoutingTable(Ptr<Node> aNode, const string& outputDir, const string& filePrefix);

Ptr<Ipv4StaticRouting> GetNodeStaticRoutingProtocol(Ptr<Node> node);

};
