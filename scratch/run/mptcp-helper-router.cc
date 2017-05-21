#include "ns3/internet-module.h"

namespace ns3{

void PrintRoutingTable(Ptr<Node> aNode, const string& outputDir, const string& filePrefix)
{
  //Print the node's routing table
  Ptr<Ipv4> switchIpv4 = aNode->GetObject<Ipv4> ();
  Ptr<Ipv4RoutingProtocol> routingProtocol = switchIpv4->GetRoutingProtocol();
  Ptr<Ipv4ListRouting> listRouting = DynamicCast<Ipv4ListRouting>(routingProtocol);
  int16_t priority;
  // Ptr<Ipv4GlobalRouting> routing = DynamicCast<Ipv4GlobalRouting>(listRouting->GetRoutingProtocol(0, priority));

  Ptr<Ipv4StaticRouting> routing = DynamicCast<Ipv4StaticRouting>(listRouting->GetRoutingProtocol(0, priority));

  stringstream rtablePath;
  rtablePath << outputDir << "/mptcp_" << filePrefix << "_routing_table";
  Ptr<OutputStreamWrapper> rtableFile = Create<OutputStreamWrapper>(rtablePath.str(), std::ios::out);

  routing->PrintRoutingTable(rtableFile);
}

Ptr<Ipv4StaticRouting> GetNodeStaticRoutingProtocol(Ptr<Node> node)
{
  Ptr<Ipv4> nodeIpv4 = node->GetObject<Ipv4> ();
  Ptr<Ipv4RoutingProtocol> routingProtocol = nodeIpv4->GetRoutingProtocol();
  Ptr<Ipv4ListRouting> listRouting = DynamicCast<Ipv4ListRouting>(routingProtocol);
  int16_t priority;
  Ptr<Ipv4StaticRouting> routing = DynamicCast<Ipv4StaticRouting>(listRouting->GetRoutingProtocol(0, priority));
  return routing;
}

};
