#include <stdio.h>
#include <string>
#include <limits>
#include "mptcp-helper-os.h"
#include "mp-onoff-application.h"

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include <sys/stat.h>
#include <unistd.h>
#include "file-transfer-helper.h"
#include "file-transfer-application.h"
#include <vector>

namespace ns3{

Ptr<Ipv4StaticRouting> GetNodeStaticRoutingProtocol(Ptr<Node> node)
{
  Ptr<Ipv4> nodeIpv4 = node->GetObject<Ipv4> ();
  Ptr<Ipv4RoutingProtocol> routingProtocol = nodeIpv4->GetRoutingProtocol();
  Ptr<Ipv4ListRouting> listRouting = DynamicCast<Ipv4ListRouting>(routingProtocol);
  int16_t priority;
  Ptr<Ipv4StaticRouting> routing = DynamicCast<Ipv4StaticRouting>(listRouting->GetRoutingProtocol(0, priority));
  return routing;
}

void PopulateServerRoutingTable(Ptr<Node> aServer,
                                Ipv4InterfaceContainer& clientInterfaces,
                                Ipv4InterfaceContainer& switchClientInterfaces,
                                Ipv4InterfaceContainer& switchServerInterfaces)
{
  Ptr<Ipv4StaticRouting> routing = GetNodeStaticRoutingProtocol(aServer);

  //Routes to switch interfaces facing the server
  for(uint32_t i = 0; i < switchServerInterfaces.GetN(); ++i)
  {
    routing->AddHostRouteTo(switchServerInterfaces.GetAddress(i), switchServerInterfaces.GetAddress(i), i + 1);
  }

  uint32_t numClientInterfaces = switchClientInterfaces.GetN();
  uint32_t numServerInterfaces = switchServerInterfaces.GetN();

  for(uint32_t i = 0; i < numClientInterfaces; ++i)
  {
    if (numClientInterfaces == 1)
    {
      for (uint32_t j = 0; j < numServerInterfaces; ++j)
      {
        routing->AddHostRouteTo(switchClientInterfaces.GetAddress(i), switchServerInterfaces.GetAddress(j), j + 1);
      }
    }
    else if (numClientInterfaces == numServerInterfaces)
    {
      routing->AddHostRouteTo(switchClientInterfaces.GetAddress(i), switchServerInterfaces.GetAddress(i), i + 1);
    }
  }

  for(uint32_t i = 0; i < clientInterfaces.GetN(); ++i)
  {
    if (numClientInterfaces == 1)
    {
      for (uint32_t j = 0; j < switchServerInterfaces.GetN(); ++j)
      {
        routing->AddHostRouteTo(clientInterfaces.GetAddress(i), switchServerInterfaces.GetAddress(j), j + 1);
      }
    }
    else if (numClientInterfaces == numServerInterfaces)
    {
      routing->AddHostRouteTo(clientInterfaces.GetAddress(i), switchServerInterfaces.GetAddress(i), i + 1);
    }
  }
}

void PopulateSwitchRoutingTable(Ptr<Node> aSwitch,
                                Ipv4InterfaceContainer& clientInterfaces,
                                Ipv4InterfaceContainer& serverInterfaces,
                                Ipv4InterfaceContainer& switchClientInterfaces,
                                Ipv4InterfaceContainer& switchServerInterfaces)
{
  Ptr<Ipv4StaticRouting> routing = GetNodeStaticRoutingProtocol(aSwitch);

  uint32_t numSwitchServerInterfaces = switchServerInterfaces.GetN();
  for(uint32_t i = 0; i < serverInterfaces.GetN(); ++i)
  {
    //Routes to the server interfaces facing the server
    routing->AddHostRouteTo(serverInterfaces.GetAddress(i),
                            serverInterfaces.GetAddress(i), i + 1);
  }

  //Routes to the client
  for(uint32_t i = 0; i < clientInterfaces.GetN(); ++i)
  {
    routing->AddHostRouteTo(clientInterfaces.GetAddress(i), clientInterfaces.GetAddress(i), numSwitchServerInterfaces + i+1);
  }
}

void PopulateClientRoutingTable(Ptr<Node> aClient,
                                Ipv4InterfaceContainer& serverInterfaces,
                                Ipv4InterfaceContainer& switchClientInterfaces,
                                Ipv4InterfaceContainer& switchServerInterfaces)
{
  uint32_t numClientInterfaces = switchClientInterfaces.GetN();

  if (numClientInterfaces == 1)
  {
    //nothing to do, just one interface
    return;
  }

  Ptr<Ipv4StaticRouting> routing = GetNodeStaticRoutingProtocol(aClient);

  //Routes to switch interfaces facing the client
  for(uint32_t i = 0; i < numClientInterfaces; ++i)
  {
    routing->AddHostRouteTo(switchClientInterfaces.GetAddress(i), switchClientInterfaces.GetAddress(i), i + 1);
  }

  uint32_t numServerInterfaces = serverInterfaces.GetN();

  NS_ASSERT(numClientInterfaces == numServerInterfaces);

  for(uint32_t i = 0; i < numServerInterfaces; ++i)
  {
    routing->AddHostRouteTo(switchServerInterfaces.GetAddress(i), switchClientInterfaces.GetAddress(i), i + 1);
  }

  for(uint32_t i = 0; i < serverInterfaces.GetN(); ++i)
  {
    routing->AddHostRouteTo(serverInterfaces.GetAddress(i), switchClientInterfaces.GetAddress(i), i + 1);
  }

}

void PrintRoutingTable(Ptr<Node> aNode, const string& outputDir, const string& filePrefix)
{
  //Print the node's routing table
  Ptr<Ipv4> switchIpv4 = aNode->GetObject<Ipv4> ();
  Ptr<Ipv4RoutingProtocol> routingProtocol = switchIpv4->GetRoutingProtocol();
  Ptr<Ipv4ListRouting> listRouting = DynamicCast<Ipv4ListRouting>(routingProtocol);
  int16_t priority;
  //Ptr<Ipv4GlobalRouting> routing = DynamicCast<Ipv4GlobalRouting>(listRouting->GetRoutingProtocol(0, priority));

  Ptr<Ipv4StaticRouting> routing = DynamicCast<Ipv4StaticRouting>(listRouting->GetRoutingProtocol(0, priority));

  stringstream rtablePath;
  rtablePath << outputDir << "/mptcp_" << filePrefix << "_routing_table";
  Ptr<OutputStreamWrapper> rtableFile = Create<OutputStreamWrapper>(rtablePath.str(), std::ios::out);

  routing->PrintRoutingTable(rtableFile);
}

};
