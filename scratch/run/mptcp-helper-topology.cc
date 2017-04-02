#include "mptcp-helper-router.h"

#include "ns3/netanim-module.h"
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
  // void ns3::Ipv4ListRoutingHelper::Add(const Ipv4RoutingHelper & routing, int16_t priority)
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
  /*
   * Server has two possible paths to client, there is a bottleneck link at switch.
   *
   ------           ------           ------
  |      | ------  |      |-------  |      |
  |      | ------  |      |         |      |
   ------           ------           ------
   Server           Switch           Client
   *
   */

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

void CreateRealNetwork (uint32_t packetSize,
                        NodeContainer& server,
                        NodeContainer& client,
                        NodeContainer& isps,
                        NodeContainer& ixs,
                        Ipv4Address& remoteClient)
{
  //Create the internet stack helper.
  InternetStackHelper stackHelper = GetInternetStackHelper();

  client.Create(1);           // client is connected to Japan
  stackHelper.Install(client);

  server.Create(1);           // server is connected to Beijin
  stackHelper.Install(server);

  isps.Create(6);             // ICB, EDU, CU, CT, CM and CST
  stackHelper.Install(isps);

  ixs.Create(2);              // Beijing and Japan
  stackHelper.Install(ixs);

  Ptr<Node> isp_icb = isps.Get(0);
  Ptr<Node> isp_edu = isps.Get(1);
  Ptr<Node> isp_cu = isps.Get(2);
  Ptr<Node> isp_ct = isps.Get(3);
  Ptr<Node> isp_cm = isps.Get(4);
  Ptr<Node> isp_cst = isps.Get(5);
  Ptr<Node> ix_Beijing = ixs.Get(0);
  Ptr<Node> ix_Japan = ixs.Get(1);

  AnimationInterface::SetConstantPosition	(isps.Get(0), 200, 0);
  AnimationInterface::SetConstantPosition	(isps.Get(1), 100, 200);
  AnimationInterface::SetConstantPosition	(isps.Get(2), 200, 400);
  AnimationInterface::SetConstantPosition	(isps.Get(3), 100, 600);
  AnimationInterface::SetConstantPosition	(isps.Get(4), 200, 800);
  AnimationInterface::SetConstantPosition	(isps.Get(5), 100, 1000);
  AnimationInterface::SetConstantPosition	(ixs.Get(0), 500, 300);
  AnimationInterface::SetConstantPosition	(ixs.Get(1), 500, 700);
  AnimationInterface::SetConstantPosition	(server.Get(0), 600, 300);
  AnimationInterface::SetConstantPosition	(client.Get(0), 600, 700);
  /*--------------------------------------*/

  //Create the address helper
  Ipv4AddressHelper addressHelper;
  addressHelper.SetBase("10.10.0.0", "255.255.255.0");

  Ipv4InterfaceContainer clientInterfaces;
  Ipv4InterfaceContainer serverInterfaces;
  Ipv4InterfaceContainer ispInterfaces_icb;
  Ipv4InterfaceContainer ispInterfaces_edu;
  Ipv4InterfaceContainer ispInterfaces_cu;
  Ipv4InterfaceContainer ispInterfaces_ct;
  Ipv4InterfaceContainer ispInterfaces_cm;
  Ipv4InterfaceContainer ispInterfaces_cst;
  Ipv4InterfaceContainer ixsInterfaces_Beijin;
  Ipv4InterfaceContainer ixsInterfaces_Japan;

  Ipv4InterfaceContainer interfaces;

  /*------------isp with isp-----------------*/
  addressHelper.SetBase("10.10.0.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(isp_icb, isp_cu, DataRate("310Mbps"), Time("5ms"), packetSize));
  ispInterfaces_icb.Add(interfaces.Get(0));
  ispInterfaces_cu.Add(interfaces.Get(1));
  // NetDeviceContainer dev = PointToPointCreate(isp_icb, isp_cu, DataRate("310Mbps"), Time("5ms"), packetSize);
  // interfaces = addressHelper.Assign(dev);
  // ispInterfaces_icb.Add(interfaces.Get(0));
  // ispInterfaces_cu.Add(interfaces.Get(1));
  //
  // std::pair<Ptr<Ipv4>, uint32_t> returnValue = interfaces.Get(1);
  // Ptr<Ipv4> ipv4 = returnValue.first;
  // uint32_t index = returnValue.second;
  // Ptr<Ipv4Interface> iface =  ipv4->GetObject<Ipv4L3Protocol> ()->GetInterface (index);
  // uint32_t address = iface->GetAddress(0).GetLocal().Get();
  // cout << "Jiaming Hong: " << ((address >> 24) & 0xff) << "."
  //                          << ((address >> 16) & 0xff) << "."
  //                          << ((address >> 8) & 0xff) << "."
  //                          << ((address >> 0) & 0xff) << endl;
  addressHelper.SetBase("10.10.1.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(isp_icb, isp_ct, DataRate("100Mbps"), Time("5ms"), packetSize));
  ispInterfaces_icb.Add(interfaces.Get(0));
  ispInterfaces_ct.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.2.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(isp_edu, isp_cst, DataRate("11000Mbps"), Time("5ms"), packetSize));
  ispInterfaces_edu.Add(interfaces.Get(0));
  ispInterfaces_cst.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.3.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(isp_edu, isp_cm, DataRate("20692Mbps"), Time("5ms"), packetSize));
  ispInterfaces_edu.Add(interfaces.Get(0));
  ispInterfaces_cm.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.4.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(isp_edu, isp_ct, DataRate("27000Mbps"), Time("5ms"), packetSize));
  ispInterfaces_edu.Add(interfaces.Get(0));
  ispInterfaces_ct.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.5.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(isp_edu, isp_cu, DataRate("27000Mbps"), Time("5ms"), packetSize));
  ispInterfaces_edu.Add(interfaces.Get(0));
  ispInterfaces_cu.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.6.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(isp_cu, isp_cst, DataRate("5000Mbps"), Time("5ms"), packetSize));
  ispInterfaces_cu.Add(interfaces.Get(0));
  ispInterfaces_cst.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.7.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(isp_cu, isp_cm, DataRate("91024Mbps"), Time("5ms"), packetSize));
  ispInterfaces_cu.Add(interfaces.Get(0));
  ispInterfaces_cm.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.8.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(isp_cu, isp_ct, DataRate("747000Mbps"), Time("5ms"), packetSize));
  ispInterfaces_cu.Add(interfaces.Get(0));
  ispInterfaces_ct.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.9.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(isp_ct, isp_cm, DataRate("137168Mbps"), Time("5ms"), packetSize));
  ispInterfaces_ct.Add(interfaces.Get(0));
  ispInterfaces_cm.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.10.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(isp_ct, isp_cst, DataRate("5600Mbps"), Time("5ms"), packetSize));
  ispInterfaces_ct.Add(interfaces.Get(0));
  ispInterfaces_cst.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.11.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(isp_cm, isp_cst, DataRate("2500Mbps"), Time("5ms"), packetSize));
  ispInterfaces_cm.Add(interfaces.Get(0));
  ispInterfaces_cst.Add(interfaces.Get(1));

  /*-------- isp with ix --------------*/
  addressHelper.SetBase("10.10.12.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(ix_Japan, isp_edu, DataRate("2500Mbps"), Time("5ms"), packetSize));
  ixsInterfaces_Japan.Add(interfaces.Get(0));
  ispInterfaces_edu.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.13.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(ix_Japan, isp_cu, DataRate("2500Mbps"), Time("5ms"), packetSize));
  ixsInterfaces_Japan.Add(interfaces.Get(0));
  ispInterfaces_cu.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.14.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(ix_Japan, isp_ct, DataRate("2500Mbps"), Time("5ms"), packetSize));
  ixsInterfaces_Japan.Add(interfaces.Get(0));
  ispInterfaces_ct.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.15.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(ix_Japan, isp_cm, DataRate("2500Mbps"), Time("5ms"), packetSize));
  ixsInterfaces_Japan.Add(interfaces.Get(0));
  ispInterfaces_cm.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.16.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(ix_Japan, isp_cst, DataRate("2500Mbps"), Time("5ms"), packetSize));
  ixsInterfaces_Japan.Add(interfaces.Get(0));
  ispInterfaces_cst.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.17.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(ix_Beijing, isp_icb, DataRate("2500Mbps"), Time("5ms"), packetSize));
  ixsInterfaces_Beijin.Add(interfaces.Get(0));
  ispInterfaces_icb.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.18.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(ix_Beijing, isp_edu, DataRate("2500Mbps"), Time("5ms"), packetSize));
  ixsInterfaces_Beijin.Add(interfaces.Get(0));
  ispInterfaces_edu.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.19.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(ix_Beijing, isp_cu, DataRate("2500Mbps"), Time("5ms"), packetSize));
  ixsInterfaces_Beijin.Add(interfaces.Get(0));
  ispInterfaces_cu.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.20.0", "255.255.255.0");

  interfaces = addressHelper.Assign(PointToPointCreate(ix_Beijing, isp_ct, DataRate("2500Mbps"), Time("5ms"), packetSize));
  ixsInterfaces_Beijin.Add(interfaces.Get(0));
  ispInterfaces_ct.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.21.0", "255.255.255.0");

  interfaces = addressHelper.Assign(PointToPointCreate(ix_Beijing, isp_cm, DataRate("2500Mbps"), Time("5ms"), packetSize));
  ixsInterfaces_Beijin.Add(interfaces.Get(0));
  ispInterfaces_cm.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.22.0", "255.255.255.0");

  interfaces = addressHelper.Assign(PointToPointCreate(ix_Beijing, isp_cst, DataRate("2500Mbps"), Time("5ms"), packetSize));
  ixsInterfaces_Beijin.Add(interfaces.Get(0));
  ispInterfaces_cst.Add(interfaces.Get(1));

  /*-------- server with ix --------------*/

  addressHelper.SetBase("10.10.23.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(client.Get(0), ix_Japan, DataRate("2500Mbps"), Time("5ms"), packetSize));
  clientInterfaces.Add(interfaces.Get(0));
  ixsInterfaces_Japan.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.24.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(client.Get(0), ix_Japan, DataRate("2500Mbps"), Time("5ms"), packetSize));
  clientInterfaces.Add(interfaces.Get(0));
  ixsInterfaces_Japan.Add(interfaces.Get(1));

  /*-------- client with ix --------------*/
  addressHelper.SetBase("10.10.25.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(server.Get(0), ix_Beijing, DataRate("2500Mbps"), Time("5ms"), packetSize));
  serverInterfaces.Add(interfaces.Get(0));
  ixsInterfaces_Beijin.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.26.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(server.Get(0), ix_Beijing, DataRate("2500Mbps"), Time("5ms"), packetSize));
  serverInterfaces.Add(interfaces.Get(0));
  ixsInterfaces_Beijin.Add(interfaces.Get(1));

  remoteClient = clientInterfaces.GetAddress(0); // important, do not forget!
}

};
