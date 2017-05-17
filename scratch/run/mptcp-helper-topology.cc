#include <utility>
#include <vector>

#include "mptcp-helper-router.h"

#include "ns3/core-module.h"
#include "ns3/netanim-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/traffic-control-module.h"

#include "ns3/aodv-module.h"
// #include "ns3/olsr-module.h"

namespace ns3{

InternetStackHelper GetInternetStackHelper (bool useStaticRouting)
{
  // Create the internet stack helper, and install the internet stack on the client node
  // Tried olsr, dsdv, but neither of them work
  InternetStackHelper stackHelper;

  if(useStaticRouting == false){
    AodvHelper aodv;
    // OlsrHelper olsr;
    Ipv4ListRoutingHelper listRoutingHelper;
    listRoutingHelper.Add (aodv, 10);
    // listRoutingHelper.Add (olsr, 100);
    stackHelper.SetRoutingHelper (listRoutingHelper);
  }
  else{
    //Set the routing protocol to static routing
    Ipv4ListRoutingHelper listRoutingHelper;
    Ipv4StaticRoutingHelper staticRoutingHelper;
    listRoutingHelper.Add(staticRoutingHelper, 10);
    stackHelper.SetRoutingHelper(listRoutingHelper);
  }
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
                        "MaxPackets", UintegerValue(10));
  pointToPoint.EnablePcapAll ("mptcp");
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
  // addressHelper.SetBase("192.168.0.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces;

  /*------------isp with isp-----------------*/
  addressHelper.SetBase("192.168.0.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(isp_icb, isp_cu, DataRate("310Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.1.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(isp_icb, isp_ct, DataRate("100Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.2.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(isp_edu, isp_cst, DataRate("11000Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.3.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(isp_edu, isp_cm, DataRate("20692Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.4.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(isp_edu, isp_ct, DataRate("27000Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.5.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(isp_edu, isp_cu, DataRate("27000Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.6.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(isp_cu, isp_cst, DataRate("5000Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.7.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(isp_cu, isp_cm, DataRate("91024Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.8.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(isp_cu, isp_ct, DataRate("747000Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.9.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(isp_ct, isp_cm, DataRate("137168Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.10.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(isp_ct, isp_cst, DataRate("5600Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.11.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(isp_cm, isp_cst, DataRate("2500Kbps"), Time("5ms"), packetSize));

  /*-------- isp with ix --------------*/
  addressHelper.SetBase("192.168.12.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(ix_Japan, isp_edu, DataRate("10000Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.13.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(ix_Japan, isp_cu, DataRate("139050Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.14.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(ix_Japan, isp_ct, DataRate("139551Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.15.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(ix_Japan, isp_cm, DataRate("31000Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.16.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(ix_Japan, isp_cst, DataRate("1000Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.17.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(ix_Beijing, isp_icb, DataRate("1000Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.18.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(ix_Beijing, isp_edu, DataRate("1000Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.19.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(ix_Beijing, isp_cu, DataRate("22000Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.20.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(ix_Beijing, isp_ct, DataRate("10000Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.21.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(ix_Beijing, isp_cm, DataRate("41720Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.22.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(ix_Beijing, isp_cst, DataRate("1000Kbps"), Time("5ms"), packetSize));
  /*-------- client with ix --------------*/

  addressHelper.SetBase("192.168.23.0", "255.255.255.0");
  NetDeviceContainer d0_client0_Japan = PointToPointCreate(client.Get(0), ix_Japan, DataRate("1Kbps"), Time("5ms"), packetSize);
  addressHelper.Assign(d0_client0_Japan);

  addressHelper.SetBase("192.168.24.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(client.Get(0), ix_Japan, DataRate("1Kbps"), Time("5ms"), packetSize));

  /*-------- server with ix --------------*/
  addressHelper.SetBase("192.168.25.0", "255.255.255.0");
  NetDeviceContainer d0_server0_Beijing = PointToPointCreate(server.Get(0), ix_Beijing, DataRate("1Kbps"), Time("5ms"), packetSize);
  addressHelper.Assign(d0_server0_Beijing);

  addressHelper.SetBase("192.168.26.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(server.Get(0), ix_Beijing, DataRate("1Kbps"), Time("5ms"), packetSize));

  other_servers.Add(isp_icb);
  other_servers.Add(isp_edu);
  // other_servers.Add(isp_cu);
  // other_clients.Add(isp_ct);
  other_clients.Add(isp_cm);
  other_clients.Add(isp_cst);

  // Ptr<RateErrorModel> ptr_em = CreateObjectWithAttributes<RateErrorModel> ();
  // ptr_em->SetRate(0.0001);
  // // d0_client0_Japan.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue (ptr_em));
  // d0_server0_Beijing.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue (ptr_em));

  // addressHelper.SetBase("192.168.99.0", "255.255.255.0");
  // addressHelper.Assign(PointToPointCreate(other_clients.Get(0), other_servers.Get(0), DataRate("1Kbps"), Time("5ms"), packetSize));
  // addressHelper.Assign(PointToPointCreate(isps.Get(0), isps.Get(5), DataRate("1000Kbps"), Time("5ms"), packetSize));
  // addressHelper.Assign(PointToPointCreate(isp_icb, isp_cst, DataRate("1000Kbps"), Time("5ms"), packetSize));

  // Ptr<Ipv4Interface> iface =  other_clients.Get(0)->GetObject<Ipv4>()->GetObject<Ipv4L3Protocol>()->GetInterface (2);
  // uint32_t address = iface->GetAddress(0).GetLocal().Get();
  // cout << "--------Jiaming Hong Debug 2: node id: " << other_clients.Get(0)->GetId() << " ip: " << ((address >> 24) & 0xff) << "."
  //                          << ((address >> 16) & 0xff) << "."
  //                          << ((address >> 8) & 0xff) << "."
  //                          << ((address >> 0) & 0xff) << endl;


}


void CreateSimplestNetwork (uint32_t packetSize,
                        NodeContainer& server,
                        NodeContainer& client,
                        NodeContainer& middle,
                        NodeContainer& other_servers,
                        NodeContainer& other_clients)
{
  //                        ----
  //                      /      \
  // (1, server) B ---  C        A (0, client)
  //                     \      /
  //                       ----

  // #include "ns3/random-variable-stream.h"
  // Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();

  //Create the internet stack helper.
  client.Create(1);           // A, Alice
  stackHelper.Install(client);

  server.Create(1);           // B, Bob
  stackHelper.Install(server);

  middle.Create(1);             // C
  stackHelper.Install(middle);

  Ptr<Node> A = client.Get(0);
  Ptr<Node> B = server.Get(0);
  Ptr<Node> C = middle.Get(0);

  AnimationInterface::SetConstantPosition	(B, 0, 200);
  AnimationInterface::SetConstantPosition	(C, 200, 200);
  AnimationInterface::SetConstantPosition	(A, 400, 200);

  bool useStaticRouting = true;
  if(useStaticRouting == false){
    InternetStackHelper stackHelper = GetInternetStackHelper(useStaticRouting); // default value is false

    //Create the address helper
    Ipv4AddressHelper addressHelper;
    // addressHelper.SetBase("192.168.0.0", "255.255.255.0");

    Ipv4InterfaceContainer interfaces;

    addressHelper.SetBase("192.168.0.0", "255.255.255.0");
    addressHelper.Assign(PointToPointCreate(B, C, DataRate("300Kbps"), Time("6ms"), packetSize));

    addressHelper.SetBase("192.168.9.0", "255.255.255.0");
    addressHelper.Assign(PointToPointCreate(C, A, DataRate("100Kbps"), Time("500ms"), packetSize));

    addressHelper.SetBase("192.168.11.0", "255.255.255.0");
    addressHelper.Assign(PointToPointCreate(C, A, DataRate("100Kbps"), Time("15ms"), packetSize));
  }
  else{
    // TODO(Hong Jiaming): static routing not finished
    InternetStackHelper stackHelper = GetInternetStackHelper(useStaticRouting);

    //Create the address helper
    Ipv4AddressHelper addressHelper;
    // addressHelper.SetBase("192.168.0.0", "255.255.255.0");

    Ipv4InterfaceContainer interfaces;

    addressHelper.SetBase("192.168.0.0", "255.255.255.0");
    addressHelper.Assign(PointToPointCreate(B, C, DataRate("300Kbps"), Time("6ms"), packetSize));

    addressHelper.SetBase("192.168.9.0", "255.255.255.0");
    addressHelper.Assign(PointToPointCreate(C, A, DataRate("100Kbps"), Time("500ms"), packetSize));

    addressHelper.SetBase("192.168.11.0", "255.255.255.0");
    addressHelper.Assign(PointToPointCreate(C, A, DataRate("100Kbps"), Time("15ms"), packetSize));

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
  }
}

void CreateClassicNetwork (uint32_t packetSize,
                        NodeContainer& server,
                        NodeContainer& client,
                        NodeContainer& middle,
                        NodeContainer& other_servers,
                        NodeContainer& other_clients)
{
  //Create the internet stack helper.
  InternetStackHelper stackHelper = GetInternetStackHelper();

  client.Create(1);           // A
  stackHelper.Install(client);

  server.Create(1);           // D
  stackHelper.Install(server);

  middle.Create(4);             // B, C, E, F
  stackHelper.Install(middle);

  Ptr<Node> A = client.Get(0);
  Ptr<Node> B = middle.Get(0);
  Ptr<Node> C = middle.Get(1);
  Ptr<Node> D = server.Get(0);
  Ptr<Node> E = middle.Get(2);
  Ptr<Node> F = middle.Get(3);

  other_servers.Add(B);
  other_servers.Add(E);
  other_clients.Add(F);
  other_clients.Add(C);

  AnimationInterface::SetConstantPosition	(A, 0, 400);
  AnimationInterface::SetConstantPosition	(B, 200, 200);
  AnimationInterface::SetConstantPosition	(C, 400, 200);
  AnimationInterface::SetConstantPosition	(D, 600, 400);
  AnimationInterface::SetConstantPosition	(E, 400, 600);
  AnimationInterface::SetConstantPosition	(F, 200, 600);
  /*--------------------------------------*/

  //Create the address helper
  Ipv4AddressHelper addressHelper;
  // addressHelper.SetBase("192.168.0.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces;

  addressHelper.SetBase("192.168.0.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(A, B, DataRate("1000Kbps"), Time("20ms"), packetSize));

  addressHelper.SetBase("192.168.1.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(A, F, DataRate("500Kbps"), Time("2ms"), packetSize)); // Not used

  addressHelper.SetBase("192.168.4.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(B, C, DataRate("700Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.5.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(B, E, DataRate("300Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.6.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(F, C, DataRate("200Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.7.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(F, E, DataRate("300Kbps"), Time("5ms"), packetSize));

  // addressHelper.SetBase("192.168.9.0", "255.255.255.0");
  // addressHelper.Assign(PointToPointCreate(C, D, DataRate("200Kbps"), Time("4.55ms"), packetSize));

  // Hong Jiaming: Very strange, if I change the delay of this link from 4.55ms to 4.54ms , simulation changes dramatically.
  // I suspect that the packets of aodv routing protocol may colliside in simulation if delay is too close.
  // Or the protocol tends to pick the best path, while all the path performs extremely similar.
  // I don't know whether this happens in real world or not.
  addressHelper.SetBase("192.168.9.0", "255.255.255.0");
  NetDeviceContainer d_CD = PointToPointCreate(C, D, DataRate("200Kbps"), Time("4ms"), packetSize);
  addressHelper.Assign(d_CD);

  addressHelper.SetBase("192.168.11.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(E, D, DataRate("800Kbps"), Time("8ms"), packetSize));


  Ptr<RateErrorModel> ptr_em = CreateObjectWithAttributes<RateErrorModel> ();
  ptr_em->SetRate(2*1e-5);
  // d0_client0_Japan.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue (ptr_em));
  d_CD.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue (ptr_em));
  d_CD.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue (ptr_em));


  // Ptr<RateErrorModel> ptr_em = CreateObjectWithAttributes<RateErrorModel> ();
  // ptr_em->SetRate(0.0001);
  // // d0_client0_Japan.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue (ptr_em));
  // d0_server0_Beijing.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue (ptr_em));
  //
  // addressHelper.SetBase("192.168.99.0", "255.255.255.0");
  // addressHelper.Assign(PointToPointCreate(other_clients.Get(0), other_servers.Get(0), DataRate("1Kbps"), Time("5ms"), packetSize));
  // addressHelper.Assign(PointToPointCreate(isps.Get(0), isps.Get(5), DataRate("1000Kbps"), Time("5ms"), packetSize));
  // addressHelper.Assign(PointToPointCreate(isp_icb, isp_cst, DataRate("1000Kbps"), Time("5ms"), packetSize));

  // Ptr<Ipv4Interface> iface =  other_clients.Get(0)->GetObject<Ipv4>()->GetObject<Ipv4L3Protocol>()->GetInterface (2);
  // uint32_t address = iface->GetAddress(0).GetLocal().Get();
  // cout << "--------Jiaming Hong Debug 2: node id: " << other_clients.Get(0)->GetId() << " ip: " << ((address >> 24) & 0xff) << "."
  //                          << ((address >> 16) & 0xff) << "."
  //                          << ((address >> 8) & 0xff) << "."
  //                          << ((address >> 0) & 0xff) << endl;
}

void CreateExtendedClassicNetwork (uint32_t packetSize,
                        NodeContainer& server,
                        NodeContainer& client,
                        NodeContainer& middle,
                        NodeContainer& other_servers,
                        NodeContainer& other_clients)
{
  //Create the internet stack helper.
  InternetStackHelper stackHelper = GetInternetStackHelper();

  client.Create(1);           // A
  stackHelper.Install(client);

  server.Create(1);           // D
  stackHelper.Install(server);

  middle.Create(4);             // B, C, E, F
  stackHelper.Install(middle);

  other_clients.Create(2);              // H, J
  stackHelper.Install(other_clients);

  other_servers.Create(2);              // G, I
  stackHelper.Install(other_servers);

  Ptr<Node> A = client.Get(0);
  Ptr<Node> B = middle.Get(0);
  Ptr<Node> C = middle.Get(1);
  Ptr<Node> D = server.Get(0);
  Ptr<Node> E = middle.Get(2);
  Ptr<Node> F = middle.Get(3);
  Ptr<Node> G = other_servers.Get(0);
  Ptr<Node> H = other_clients.Get(1);
  Ptr<Node> I = other_clients.Get(0);
  Ptr<Node> J = other_servers.Get(1);

  AnimationInterface::SetConstantPosition	(A, 0, 400);
  AnimationInterface::SetConstantPosition	(B, 200, 200);
  AnimationInterface::SetConstantPosition	(C, 400, 200);
  AnimationInterface::SetConstantPosition	(D, 600, 400);
  AnimationInterface::SetConstantPosition	(E, 400, 600);
  AnimationInterface::SetConstantPosition	(F, 200, 600);
  AnimationInterface::SetConstantPosition	(G, 0, 0);
  AnimationInterface::SetConstantPosition	(H, 600, 0);
  AnimationInterface::SetConstantPosition	(I, 0, 800);
  AnimationInterface::SetConstantPosition	(J, 600, 800);
  /*--------------------------------------*/

  //Create the address helper
  Ipv4AddressHelper addressHelper;
  // addressHelper.SetBase("192.168.0.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces;

  addressHelper.SetBase("192.168.0.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(A, B, DataRate("1000Kbps"), Time("20ms"), packetSize));

  addressHelper.SetBase("192.168.1.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(A, F, DataRate("500Kbps"), Time("2ms"), packetSize));

  addressHelper.SetBase("192.168.2.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(G, B, DataRate("1000Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.3.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(I, F, DataRate("1000Kbps"), Time("12ms"), packetSize));

  addressHelper.SetBase("192.168.4.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(B, C, DataRate("700Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.5.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(B, E, DataRate("300Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.6.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(F, C, DataRate("200Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.7.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(F, E, DataRate("300Kbps"), Time("5ms"), packetSize));

  addressHelper.SetBase("192.168.8.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(C, H, DataRate("1000Kbps"), Time("20ms"), packetSize));

  addressHelper.SetBase("192.168.10.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(E, J, DataRate("1000Kbps"), Time("6ms"), packetSize));

  addressHelper.SetBase("192.168.9.0", "255.255.255.0");
  NetDeviceContainer d_CD = PointToPointCreate(C, D, DataRate("2000Kbps"), Time("4ms"), packetSize);
  addressHelper.Assign(d_CD);

  // Ptr<RateErrorModel> ptr_em = CreateObjectWithAttributes<RateErrorModel> ();
  // ptr_em->SetRate(1e-10);
  // // ptr_em->SetRate(2*1e-4);
  // // d0_client0_Japan.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue (ptr_em));
  // d_CD.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue (ptr_em));
  // d_CD.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue (ptr_em));

  addressHelper.SetBase("192.168.11.0", "255.255.255.0");
  addressHelper.Assign(PointToPointCreate(E, D, DataRate("800Kbps"), Time("8ms"), packetSize));

  // addressHelper.SetBase("192.168.99.0", "255.255.255.0");
  // addressHelper.Assign(PointToPointCreate(other_clients.Get(0), other_servers.Get(0), DataRate("1Kbps"), Time("5ms"), packetSize));
  // addressHelper.Assign(PointToPointCreate(isps.Get(0), isps.Get(5), DataRate("1000Kbps"), Time("5ms"), packetSize));
  // addressHelper.Assign(PointToPointCreate(isp_icb, isp_cst, DataRate("1000Kbps"), Time("5ms"), packetSize));

  // Ptr<Ipv4Interface> iface =  other_clients.Get(0)->GetObject<Ipv4>()->GetObject<Ipv4L3Protocol>()->GetInterface (2);
  // uint32_t address = iface->GetAddress(0).GetLocal().Get();
  // cout << "--------Jiaming Hong Debug 2: node id: " << other_clients.Get(0)->GetId() << " ip: " << ((address >> 24) & 0xff) << "."
  //                          << ((address >> 16) & 0xff) << "."
  //                          << ((address >> 8) & 0xff) << "."
  //                          << ((address >> 0) & 0xff) << endl;


}

};
