#include <utility>
#include <vector>

#include "mptcp-helper-router.h"

#include "ns3/core-module.h"
#include "ns3/netanim-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/traffic-control-module.h"


namespace ns3{

InternetStackHelper GetInternetStackHelper ()
{
  //Create the internet stack helper, and install the internet stack on the client node
  InternetStackHelper stackHelper;
  // //Set the routing protocol to static routing
  // Ipv4ListRoutingHelper listRoutingHelper;
  // Ipv4StaticRoutingHelper staticRoutingHelper;
  // // void ns3::Ipv4ListRoutingHelper::Add(const Ipv4RoutingHelper & routing, int16_t priority)  THIS IS THE ROOT CAUSE OF ROUTING FAILURE!!!
  // listRoutingHelper.Add(staticRoutingHelper, 10);
  //
  // stackHelper.SetRoutingHelper(listRoutingHelper);

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

void CreateRealNetwork (uint32_t packetSize,
                        NodeContainer& server,
                        NodeContainer& client,
                        NodeContainer& isps,
                        NodeContainer& ixs,
                        NodeContainer& other_servers,
                        NodeContainer& other_clients)
                        // std::vector<std::pair<Ipv4Address, Ipv4Address>>& SCIpPairs)
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

  other_servers.Add(isp_icb);
  // other_servers.Add(isp_edu);
  // other_servers.Add(isp_cu);
  // other_clients.Add(isp_ct);
  // other_clients.Add(isp_cm);
  other_clients.Add(isp_cst);

  AnimationInterface::SetConstantPosition	(isps.Get(0), 0, 200);
  AnimationInterface::SetConstantPosition	(isps.Get(1), 200, 100);
  AnimationInterface::SetConstantPosition	(isps.Get(2), 400, 200);
  AnimationInterface::SetConstantPosition	(isps.Get(3), 600, 100);
  AnimationInterface::SetConstantPosition	(isps.Get(4), 800, 200);
  AnimationInterface::SetConstantPosition	(isps.Get(5), 1000, 100);
  AnimationInterface::SetConstantPosition	(ixs.Get(0), 300, 500);
  AnimationInterface::SetConstantPosition	(ixs.Get(1), 700, 500);
  AnimationInterface::SetConstantPosition	(server.Get(0), 300, 600);
  AnimationInterface::SetConstantPosition	(client.Get(0), 700, 600);
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
  interfaces = addressHelper.Assign(PointToPointCreate(isp_icb, isp_cu, DataRate("310Kbps"), Time("5ms"), packetSize));
  ispInterfaces_icb.Add(interfaces.Get(0));
  ispInterfaces_cu.Add(interfaces.Get(1));
  // NetDeviceContainer dev = PointToPointCreate(isp_icb, isp_cu, DataRate("310Kbps"), Time("5ms"), packetSize);
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
  interfaces = addressHelper.Assign(PointToPointCreate(isp_icb, isp_ct, DataRate("100Kbps"), Time("5ms"), packetSize));
  ispInterfaces_icb.Add(interfaces.Get(0));
  ispInterfaces_ct.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.2.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(isp_edu, isp_cst, DataRate("11000Kbps"), Time("5ms"), packetSize));
  ispInterfaces_edu.Add(interfaces.Get(0));
  ispInterfaces_cst.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.3.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(isp_edu, isp_cm, DataRate("20692Kbps"), Time("5ms"), packetSize));
  ispInterfaces_edu.Add(interfaces.Get(0));
  ispInterfaces_cm.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.4.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(isp_edu, isp_ct, DataRate("27000Kbps"), Time("5ms"), packetSize));
  ispInterfaces_edu.Add(interfaces.Get(0));
  ispInterfaces_ct.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.5.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(isp_edu, isp_cu, DataRate("27000Kbps"), Time("5ms"), packetSize));
  ispInterfaces_edu.Add(interfaces.Get(0));
  ispInterfaces_cu.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.6.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(isp_cu, isp_cst, DataRate("5000Kbps"), Time("5ms"), packetSize));
  ispInterfaces_cu.Add(interfaces.Get(0));
  ispInterfaces_cst.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.7.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(isp_cu, isp_cm, DataRate("91024Kbps"), Time("5ms"), packetSize));
  ispInterfaces_cu.Add(interfaces.Get(0));
  ispInterfaces_cm.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.8.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(isp_cu, isp_ct, DataRate("747000Kbps"), Time("5ms"), packetSize));
  ispInterfaces_cu.Add(interfaces.Get(0));
  ispInterfaces_ct.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.9.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(isp_ct, isp_cm, DataRate("137168Kbps"), Time("5ms"), packetSize));
  ispInterfaces_ct.Add(interfaces.Get(0));
  ispInterfaces_cm.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.10.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(isp_ct, isp_cst, DataRate("5600Kbps"), Time("5ms"), packetSize));
  ispInterfaces_ct.Add(interfaces.Get(0));
  ispInterfaces_cst.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.11.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(isp_cm, isp_cst, DataRate("2500Kbps"), Time("5ms"), packetSize));
  ispInterfaces_cm.Add(interfaces.Get(0));
  ispInterfaces_cst.Add(interfaces.Get(1));

  /*-------- isp with ix --------------*/
  addressHelper.SetBase("10.10.12.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(ix_Japan, isp_edu, DataRate("10000Kbps"), Time("5ms"), packetSize));
  ixsInterfaces_Japan.Add(interfaces.Get(0));
  ispInterfaces_edu.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.13.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(ix_Japan, isp_cu, DataRate("139050Kbps"), Time("5ms"), packetSize));
  ixsInterfaces_Japan.Add(interfaces.Get(0));
  ispInterfaces_cu.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.14.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(ix_Japan, isp_ct, DataRate("139551Kbps"), Time("5ms"), packetSize));
  ixsInterfaces_Japan.Add(interfaces.Get(0));
  ispInterfaces_ct.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.15.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(ix_Japan, isp_cm, DataRate("31000Kbps"), Time("5ms"), packetSize));
  ixsInterfaces_Japan.Add(interfaces.Get(0));
  ispInterfaces_cm.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.16.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(ix_Japan, isp_cst, DataRate("1000Kbps"), Time("5ms"), packetSize));
  ixsInterfaces_Japan.Add(interfaces.Get(0));
  ispInterfaces_cst.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.17.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(ix_Beijing, isp_icb, DataRate("1000Kbps"), Time("5ms"), packetSize));
  ixsInterfaces_Beijin.Add(interfaces.Get(0));
  ispInterfaces_icb.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.18.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(ix_Beijing, isp_edu, DataRate("1000Kbps"), Time("5ms"), packetSize));
  ixsInterfaces_Beijin.Add(interfaces.Get(0));
  ispInterfaces_edu.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.19.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(ix_Beijing, isp_cu, DataRate("22000Kbps"), Time("5ms"), packetSize));
  ixsInterfaces_Beijin.Add(interfaces.Get(0));
  ispInterfaces_cu.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.20.0", "255.255.255.0");

  interfaces = addressHelper.Assign(PointToPointCreate(ix_Beijing, isp_ct, DataRate("10000Kbps"), Time("5ms"), packetSize));
  ixsInterfaces_Beijin.Add(interfaces.Get(0));
  ispInterfaces_ct.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.21.0", "255.255.255.0");

  interfaces = addressHelper.Assign(PointToPointCreate(ix_Beijing, isp_cm, DataRate("41720Kbps"), Time("5ms"), packetSize));
  ixsInterfaces_Beijin.Add(interfaces.Get(0));
  ispInterfaces_cm.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.22.0", "255.255.255.0");

  interfaces = addressHelper.Assign(PointToPointCreate(ix_Beijing, isp_cst, DataRate("1000Kbps"), Time("5ms"), packetSize));
  ixsInterfaces_Beijin.Add(interfaces.Get(0));
  ispInterfaces_cst.Add(interfaces.Get(1));

  /*-------- client with ix --------------*/

  addressHelper.SetBase("10.10.23.0", "255.255.255.0");
  NetDeviceContainer d0_client0_Japan = PointToPointCreate(client.Get(0), ix_Japan, DataRate("1000Kbps"), Time("5ms"), packetSize);
  interfaces = addressHelper.Assign(d0_client0_Japan);
  clientInterfaces.Add(interfaces.Get(0));
  ixsInterfaces_Japan.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.24.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(client.Get(0), ix_Japan, DataRate("1000Kbps"), Time("5ms"), packetSize));
  clientInterfaces.Add(interfaces.Get(0));
  ixsInterfaces_Japan.Add(interfaces.Get(1));

  /*-------- server with ix --------------*/
  addressHelper.SetBase("10.10.25.0", "255.255.255.0");
  NetDeviceContainer d0_server0_Beijing = PointToPointCreate(server.Get(0), ix_Beijing, DataRate("1000Kbps"), Time("5ms"), packetSize);
  interfaces = addressHelper.Assign(d0_server0_Beijing);
  serverInterfaces.Add(interfaces.Get(0));
  ixsInterfaces_Beijin.Add(interfaces.Get(1));

  addressHelper.SetBase("10.10.26.0", "255.255.255.0");
  interfaces = addressHelper.Assign(PointToPointCreate(server.Get(0), ix_Beijing, DataRate("1000Kbps"), Time("500ms"), packetSize));
  serverInterfaces.Add(interfaces.Get(0));
  ixsInterfaces_Beijin.Add(interfaces.Get(1));

  // Ptr<RateErrorModel> ptr_em = CreateObjectWithAttributes<RateErrorModel> ();
  // ptr_em->SetRate(0.0001);
  // // d0_client0_Japan.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue (ptr_em));
  // d0_server0_Beijing.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue (ptr_em));

  // Here, only one pair is OK
  // SCIpPairs.push_back(std::make_pair(serverInterfaces.GetAddress(0) ,clientInterfaces.GetAddress(0)));
  // SCIpPairs.push_back(std::make_pair(serverInterfaces.GetAddress(1) ,clientInterfaces.GetAddress(1)));

  // remoteClient = clientInterfaces.GetAddress(0); // important, do not forget!
}

};
