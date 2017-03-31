#include "mptcp-helper-router.h"

#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/traffic-control-module.h"


namespace ns3{

InternetStackHelper GetInternetStackHelper ()
{
  //Create the internet stack helper, and install the internet stack on the client node
  InternetStackHelper stackHelper;
  //Set the routing protocol to static routing
  Ipv4ListRoutingHelper listRoutingHelper;
  Ipv4StaticRoutingHelper staticRoutingHelper;
  listRoutingHelper.Add(staticRoutingHelper, 10);

  stackHelper.SetRoutingHelper(listRoutingHelper);

  return stackHelper;
}

NetDeviceContainer PointToPointCreate(Ptr<Node> startNode,
                                      Ptr<Node> endNode,
                                      DataRate linkRate,
                                      Time delay,
                                      uint32_t packetSize)
{

  NodeContainer linkedNodes;
  linkedNodes.Add(startNode);
  linkedNodes.Add(endNode);


  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", DataRateValue (linkRate));
  pointToPoint.SetChannelAttribute ("Delay", TimeValue (delay));

  TrafficControlHelper tchRed;
  tchRed.SetRootQueueDisc ("ns3::RedQueueDisc",
                           "MeanPktSize", UintegerValue(packetSize),
                           "LinkBandwidth", DataRateValue(linkRate),
                           "LinkDelay", TimeValue(delay));

  pointToPoint.SetQueue("ns3::DropTailQueue",
                        "MaxPackets", UintegerValue(1));

  NetDeviceContainer linkedDevices;
  linkedDevices = pointToPoint.Install (linkedNodes);

  tchRed.Install(linkedDevices);

  return linkedDevices;
}

void CreateMultipleFlowsSingleBottleneck (uint32_t interfaceCount,
                                          uint32_t packetSize,
                                          DataRate linkRate,
                                          Time delay,
                                          NodeContainer& servers,
                                          NodeContainer& switches,
                                          NodeContainer& clients,
                                          Ipv4Address& remoteClient)
{
  //Create the internet stack helper.
  InternetStackHelper stackHelper = GetInternetStackHelper();

  //Create the nodes in the topology, and install the internet stack on them
  clients.Create(1);
  stackHelper.Install(clients);

  switches.Create(1);
  stackHelper.Install(switches);

  //Create the servers and install the internet stack on them
  servers.Create(1);
  stackHelper.Install(servers);

  //Create the address helper
  Ipv4AddressHelper addressHelper;
  addressHelper.SetBase("10.10.0.0", "255.255.255.0");

  Ipv4InterfaceContainer serverInterfaces;
  Ipv4InterfaceContainer switchServerInterfaces;
  Ipv4InterfaceContainer switchClientInterfaces;
  Ipv4InterfaceContainer clientInterfaces;

  for(uint32_t i = 0; i < interfaceCount; ++i)
  {
    //Create a link between the switch and the server, assign IP addresses
    NetDeviceContainer devices = PointToPointCreate(servers.Get(0), switches.Get(0),
                                                    DataRate(linkRate.GetBitRate() * 2), delay, packetSize);
    Ipv4InterfaceContainer interfaces = addressHelper.Assign(devices);

    serverInterfaces.Add(interfaces.Get(0));
    switchServerInterfaces.Add(interfaces.Get(1));
  }

  //Create a link between the switch and the client, assign IP addresses
  NetDeviceContainer linkedDevices = PointToPointCreate(clients.Get(0), switches.Get(0), linkRate, delay, packetSize);
  Ipv4InterfaceContainer interfaces = addressHelper.Assign(linkedDevices);

  clientInterfaces.Add(interfaces.Get(0));
  switchClientInterfaces.Add(interfaces.Get(1));

  //We should do static routing, because we'd like to explicitly create 2 different paths through
  //the switch.
  PopulateServerRoutingTable(servers.Get(0), clientInterfaces, switchClientInterfaces, switchServerInterfaces);
  PopulateSwitchRoutingTable(switches.Get(0), clientInterfaces, serverInterfaces,
                             switchClientInterfaces, switchServerInterfaces);
  PopulateClientRoutingTable(clients.Get(0), serverInterfaces, switchClientInterfaces, switchServerInterfaces);

  remoteClient = clientInterfaces.GetAddress(0);
}

void CreateMultipleFlowsNoBottleneck (uint32_t interfaceCount,
                                      uint32_t packetSize,
                                      DataRate linkRate,
                                      Time delay,
                                      NodeContainer& servers,
                                      NodeContainer& switches,
                                      NodeContainer& clients,
                                      Ipv4Address& remoteClient)
{
  //Create the internet stack helper.
  InternetStackHelper stackHelper = GetInternetStackHelper();

  //Create the nodes in the topology, and install the internet stack on them
  clients.Create(1);
  stackHelper.Install(clients);

  switches.Create(interfaceCount);
  stackHelper.Install(switches);

  //Create the servers and install the internet stack on them
  servers.Create(1);
  stackHelper.Install(servers);

  //Create the address helper
  Ipv4AddressHelper addressHelper;
  addressHelper.SetBase("10.10.0.0", "255.255.255.0");

  Ipv4InterfaceContainer serverInterfaces;
  Ipv4InterfaceContainer switchServerInterfaces;
  Ipv4InterfaceContainer switchClientInterfaces;
  Ipv4InterfaceContainer clientInterfaces;

  for(uint32_t i = 0; i < interfaceCount; ++i)
  {
    //Create a link between the switch and the server, assign IP addresses
    NetDeviceContainer devices = PointToPointCreate(servers.Get(0), switches.Get(i),
                                                    DataRate(linkRate.GetBitRate()), delay, packetSize);
    Ipv4InterfaceContainer interfaces = addressHelper.Assign(devices);

    serverInterfaces.Add(interfaces.Get(0));
    switchServerInterfaces.Add(interfaces.Get(1));
  }

  for(uint32_t i = 0; i < interfaceCount; ++i)
  {
    //Create a link between the switch and the client, assign IP addresses
    NetDeviceContainer linkedDevices = PointToPointCreate(clients.Get(0), switches.Get(i),
                                                          DataRate(linkRate.GetBitRate()), delay, packetSize);
    Ipv4InterfaceContainer interfaces = addressHelper.Assign(linkedDevices);

    clientInterfaces.Add(interfaces.Get(0));
    switchClientInterfaces.Add(interfaces.Get(1));
  }

  remoteClient = clientInterfaces.GetAddress(0);

  //We should do static routing, because we'd like to explicitly create 2 different paths through
  //the switch.
  PopulateServerRoutingTable(servers.Get(0), clientInterfaces, switchClientInterfaces, switchServerInterfaces);
  PopulateClientRoutingTable(clients.Get(0), serverInterfaces, switchClientInterfaces, switchServerInterfaces);

  //Do the switches manually
  for (uint32_t i =0; i < interfaceCount; ++i)
  {
    Ptr<Node> aSwitch = switches.Get(i);
    Ptr<Ipv4StaticRouting> routing = GetNodeStaticRoutingProtocol(aSwitch);

    //Routes to the server interfaces facing the server
    routing->AddHostRouteTo(serverInterfaces.GetAddress(i),
                            serverInterfaces.GetAddress(i), 1);

    //Routes to the client
    for(uint32_t j = 0; j < clientInterfaces.GetN(); ++j)
    {
      routing->AddHostRouteTo(clientInterfaces.GetAddress(j), clientInterfaces.GetAddress(j), 2);
    }

    Ptr<Node> aServer = servers.Get(0);
    routing = GetNodeStaticRoutingProtocol(aServer);
    routing->AddHostRouteTo(clientInterfaces.GetAddress(0), switchServerInterfaces.GetAddress(i), i + 1);
  }

}

void CreateMultipleAndTcpFlows (uint32_t interfaceCount,
                                uint32_t packetSize,
                                DataRate linkRate,
                                Time delay,
                                NodeContainer& servers,
                                NodeContainer& switches,
                                NodeContainer& clients,
                                Ipv4Address& remoteClient)
{
  //Create the internet stack helper.
  InternetStackHelper mptcpStackHelper = GetInternetStackHelper();

  //Create the nodes in the topology, and install the internet stack on them
  clients.Create(1);
  mptcpStackHelper.Install(clients);

  switches.Create(1);
  mptcpStackHelper.Install(switches);

  //Create the servers and install the internet stack on them
  servers.Create(2);
  mptcpStackHelper.Install(servers);

  //Create the address helper
  Ipv4AddressHelper addressHelper;
  addressHelper.SetBase("10.10.0.0", "255.255.255.0");

  Ipv4InterfaceContainer serverInterfaces;
  Ipv4InterfaceContainer switchServerInterfaces;
  Ipv4InterfaceContainer switchClientInterfaces;
  Ipv4InterfaceContainer clientInterfaces;
  Ipv4InterfaceContainer tcpServerInterfaces;
  Ipv4InterfaceContainer tcpSwitchServerInterfaces;

  for(uint32_t i = 0; i < interfaceCount; ++i)
  {
    //Create a link between the switch and the MPTCP server, assign IP addresses
    NetDeviceContainer devices = PointToPointCreate(servers.Get(0), switches.Get(0),
                                                    linkRate, delay, packetSize);
    Ipv4InterfaceContainer interfaces = addressHelper.Assign(devices);

    serverInterfaces.Add(interfaces.Get(0));
    switchServerInterfaces.Add(interfaces.Get(1));
  }

  //Create a link between the switch and the client, assign IP addresses
  NetDeviceContainer linkedDevices = PointToPointCreate(clients.Get(0), switches.Get(0), linkRate, delay, packetSize);
  Ipv4InterfaceContainer interfaces = addressHelper.Assign(linkedDevices);

  clientInterfaces.Add(interfaces.Get(0));
  switchClientInterfaces.Add(interfaces.Get(1));

  remoteClient = clientInterfaces.GetAddress(0);

  //Create a link between the switch and a normal TCP server, assign IP addresses
  NetDeviceContainer devices = PointToPointCreate(servers.Get(1), switches.Get(0), linkRate, delay, packetSize);
  interfaces = addressHelper.Assign(devices);

  tcpServerInterfaces.Add(interfaces.Get(0));
  tcpSwitchServerInterfaces.Add(interfaces.Get(1));

  //We should do static routing, because we'd like to explicitly create 2 different paths through
  //the switch.
  PopulateServerRoutingTable(servers.Get(0), clientInterfaces, switchClientInterfaces, switchServerInterfaces);
  PopulateSwitchRoutingTable(switches.Get(0), clientInterfaces, serverInterfaces,
                             switchClientInterfaces, switchServerInterfaces);
  PopulateClientRoutingTable(clients.Get(0), serverInterfaces, switchClientInterfaces, switchServerInterfaces);

  //Manually populate the routing tables with info about the normal TCP server
  Ptr<Ipv4StaticRouting> routing = GetNodeStaticRoutingProtocol(servers.Get(1));

  uint32_t serverFacingInterface = switchServerInterfaces.GetN() + switchClientInterfaces.GetN() + 1;
  //Routes to switch interfaces facing the server
  routing->AddHostRouteTo(tcpSwitchServerInterfaces.GetAddress(0),
                          tcpSwitchServerInterfaces.GetAddress(0), 1);

  //Route from the server to the client switch interface
  routing->AddHostRouteTo(switchClientInterfaces.GetAddress(0), tcpSwitchServerInterfaces.GetAddress(0), 1);
  //Route from server to the client
  routing->AddHostRouteTo(clientInterfaces.GetAddress(0), tcpSwitchServerInterfaces.GetAddress(0), 1);

  //Switch routing
  routing = GetNodeStaticRoutingProtocol(switches.Get(0));

  //Routes to the server interfaces facing the server
  routing->AddHostRouteTo(tcpServerInterfaces.GetAddress(0),
                          tcpServerInterfaces.GetAddress(0), serverFacingInterface);

  //Route from client to tcp server
  routing = GetNodeStaticRoutingProtocol(clients.Get(0));
  routing->AddHostRouteTo(tcpServerInterfaces.GetAddress(0), switchClientInterfaces.GetAddress(0), 1);
  routing->AddHostRouteTo(tcpSwitchServerInterfaces.GetAddress(0), switchClientInterfaces.GetAddress(0), 1);
}

};
