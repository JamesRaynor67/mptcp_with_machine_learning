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
extern std::string g_link_a_BW;
extern std::string g_link_b_BW;
extern std::string g_link_c_BW;
extern std::string g_link_a_delay;
extern std::string g_link_b_delay;
extern std::string g_link_c_delay;
extern double g_link_b_BER;
extern uint32_t g_router_b_buffer_size;
extern uint32_t g_router_c_buffer_size;

InternetStackHelper GetInternetStackHelper (bool useStaticRouting = false)
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
                                      uint32_t packetSize,
                                      uint32_t queueSize = 1)
{

  NodeContainer linkedNodes;
  linkedNodes.Add(startNode);
  linkedNodes.Add(endNode);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", DataRateValue (linkRate));
  pointToPoint.SetChannelAttribute ("Delay", TimeValue (delay));

  // RED queue disc: Random Early Detection. Disc stands for Disciplines
  // Packets received by the Traffic Control layer for transmission to a netdevice
  // can be passed to a queueing discipline (queue disc) to perform scheduling and policing.
  // The |ns3| term “queue disc” corresponds to what Linux calls a “qdisc”.
  // A netdevice can have a single (root) queue disc installed on it. Installing a queue
  // disc on a netdevice is not mandatory. If a netdevice does not have a queue disc installed
  // on it, the traffic control layer sends the packets directly to the netdevice.

  TrafficControlHelper tchRed;
  std::cout << "Queue size == " << queueSize << ", MeanPktSize size == " << packetSize << "\n";
  tchRed.SetRootQueueDisc ("ns3::RedQueueDisc",
                           "MeanPktSize", UintegerValue(packetSize),
                           "LinkBandwidth", DataRateValue(linkRate),
                           "LinkDelay", TimeValue(delay));

  NS_ASSERT(queueSize > 0);
  pointToPoint.SetQueue("ns3::DropTailQueue",
                        "MaxPackets", UintegerValue(queueSize));
  // pointToPoint.EnablePcapAll ("mptcp"); // This is only for debug use
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


vector<Ptr<NetDevice>> CreateSimplestNetwork (uint32_t packetSize,
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

  server.Create(1);           // B, Bob

  middle.Create(1);           // C

  vector<Ptr<NetDevice>> unstableDevices;
  Ptr<Node> A = client.Get(0);
  Ptr<Node> B = server.Get(0);
  Ptr<Node> C = middle.Get(0);

  AnimationInterface::SetConstantPosition	(B, 0, 200);
  AnimationInterface::SetConstantPosition	(C, 200, 200);
  AnimationInterface::SetConstantPosition	(A, 400, 200);

  bool useStaticRouting = true;
  if(useStaticRouting == false){
    InternetStackHelper stackHelper = GetInternetStackHelper(useStaticRouting); // default value is false
    stackHelper.Install(client);
    stackHelper.Install(server);
    stackHelper.Install(middle);

    //Create the address helper
    Ipv4AddressHelper addressHelper;
    // addressHelper.SetBase("192.168.0.0", "255.255.255.0");

    addressHelper.SetBase("192.168.0.0", "255.255.255.0");
    addressHelper.Assign(PointToPointCreate(B, C, DataRate("300Kbps"), Time("6ms"), packetSize));

    addressHelper.SetBase("192.168.9.0", "255.255.255.0");
    addressHelper.Assign(PointToPointCreate(C, A, DataRate("100Kbps"), Time("500ms"), packetSize));

    addressHelper.SetBase("192.168.11.0", "255.255.255.0");
    addressHelper.Assign(PointToPointCreate(C, A, DataRate("100Kbps"), Time("15ms"), packetSize));
  }
  else{
    InternetStackHelper stackHelper = GetInternetStackHelper(useStaticRouting);
    stackHelper.Install(client);
    stackHelper.Install(server);
    stackHelper.Install(middle);

    //Create the address helper
    Ipv4AddressHelper addressHelper;
    // addressHelper.SetBase("192.168.0.0", "255.255.255.0");

    NetDeviceContainer linkedDevices;
    Ipv4InterfaceContainer interfaces;
    Ipv4InterfaceContainer clientInterfaces;
    Ipv4InterfaceContainer routerInterfaces;
    Ipv4InterfaceContainer serverInterfaces;

    addressHelper.SetBase("192.168.0.0", "255.255.255.0");
    linkedDevices = PointToPointCreate(B, C, DataRate(g_link_a_BW), Time(g_link_a_delay), packetSize);
    interfaces = addressHelper.Assign(linkedDevices);
    serverInterfaces.Add(interfaces.Get(0));
    routerInterfaces.Add(interfaces.Get(1));

    addressHelper.SetBase("192.168.9.0", "255.255.255.0");
    linkedDevices = PointToPointCreate(C, A, DataRate(g_link_b_BW), Time(g_link_b_delay), packetSize);
    interfaces = addressHelper.Assign(linkedDevices);
    routerInterfaces.Add(interfaces.Get(0));
    clientInterfaces.Add(interfaces.Get(1));
    // unstableDevices.push_back(linkedDevices.Get(0));
    // unstableDevices.push_back(linkedDevices.Get(1));

    if(g_link_b_BER != 0){
      std::cout << "Error model installed in link B" << std::endl;
      Ptr<RateErrorModel> ptr_em = CreateObjectWithAttributes<RateErrorModel> ();
      ptr_em->SetRate(g_link_b_BER);
      linkedDevices.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue (ptr_em));
      linkedDevices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue (ptr_em));
    }

    addressHelper.SetBase("192.168.11.0", "255.255.255.0");
    linkedDevices = PointToPointCreate(C, A, DataRate(g_link_c_BW), Time(g_link_c_delay), packetSize);
    addressHelper.Assign(linkedDevices);
    interfaces = addressHelper.Assign(linkedDevices);
    routerInterfaces.Add(interfaces.Get(0));
    clientInterfaces.Add(interfaces.Get(1));
    unstableDevices.push_back(linkedDevices.Get(0));
    unstableDevices.push_back(linkedDevices.Get(1));

    //void ns3::Ipv4StaticRouting::AddHostRouteTo	(Ipv4Address dest, Ipv4Address nextHop, uint32_t interface, uint32_t metric = 0)
    // Notice that the 0th interface is bound to 127.0.0.0, and interface outgoing is from 1st.
    std::cout << "Hong jiaming 58: clientInterfaces number == " << clientInterfaces.GetN() << std::endl;
    Ptr<Ipv4StaticRouting> routing;
    routing = GetNodeStaticRoutingProtocol(A); // client
    routing->AddHostRouteTo(serverInterfaces.GetAddress(0), routerInterfaces.GetAddress(0), 1);
    routing->AddHostRouteTo(serverInterfaces.GetAddress(0), routerInterfaces.GetAddress(1), 2);

    routing = GetNodeStaticRoutingProtocol(B); // server
    routing->AddHostRouteTo(clientInterfaces.GetAddress(0), routerInterfaces.GetAddress(0), 1);
    routing->AddHostRouteTo(clientInterfaces.GetAddress(1), routerInterfaces.GetAddress(0), 1);

    routing = GetNodeStaticRoutingProtocol(C); // router
    routing->AddHostRouteTo(serverInterfaces.GetAddress(0), serverInterfaces.GetAddress(0), 1);
    routing->AddHostRouteTo(clientInterfaces.GetAddress(0), clientInterfaces.GetAddress(0), 2);
    routing->AddHostRouteTo(clientInterfaces.GetAddress(1), clientInterfaces.GetAddress(1), 3);
  }

  return unstableDevices;
}

vector<Ptr<NetDevice>> CreateNetwork5 (uint32_t packetSize,
                        NodeContainer& server,
                        NodeContainer& client,
                        NodeContainer& middle,
                        NodeContainer& other_servers,
                        NodeContainer& other_clients,
                        NetDeviceContainer& traceQueueDevices)
{
  //                        --D--
  //                      /       \
  // (1, server) B ---  C         A (0, client)
  //                     \       /
  //                       --E--

  //Create the internet stack helper.
  client.Create(1);           // A, Alice

  server.Create(1);           // B, Bob

  middle.Create(3);           // C, D, E

  vector<Ptr<NetDevice>> unstableDevices;
  Ptr<Node> A = client.Get(0);
  Ptr<Node> B = server.Get(0);
  Ptr<Node> C = middle.Get(0);
  Ptr<Node> D = middle.Get(1);
  Ptr<Node> E = middle.Get(2);

  AnimationInterface::SetConstantPosition	(B, 0, 200);
  AnimationInterface::SetConstantPosition	(C, 200, 200);
  AnimationInterface::SetConstantPosition	(A, 600, 200);
  AnimationInterface::SetConstantPosition	(D, 400, 100);
  AnimationInterface::SetConstantPosition	(E, 400, 300);

  bool useStaticRouting = true;
  if(useStaticRouting == false){
    InternetStackHelper stackHelper = GetInternetStackHelper(useStaticRouting); // default value is false
    stackHelper.Install(client);
    stackHelper.Install(server);
    stackHelper.Install(middle);

    //Create the address helper
    Ipv4AddressHelper addressHelper;
    // addressHelper.SetBase("192.168.0.0", "255.255.255.0");

    addressHelper.SetBase("192.168.0.0", "255.255.255.0");
    addressHelper.Assign(PointToPointCreate(B, C, DataRate("300Kbps"), Time("6ms"), packetSize));

    addressHelper.SetBase("192.168.9.0", "255.255.255.0");
    addressHelper.Assign(PointToPointCreate(C, A, DataRate("100Kbps"), Time("500ms"), packetSize));

    addressHelper.SetBase("192.168.11.0", "255.255.255.0");
    addressHelper.Assign(PointToPointCreate(C, A, DataRate("100Kbps"), Time("15ms"), packetSize));
  }
  else{
    InternetStackHelper stackHelper = GetInternetStackHelper(useStaticRouting);
    stackHelper.Install(client);
    stackHelper.Install(server);
    stackHelper.Install(middle);

    //Create the address helper
    Ipv4AddressHelper addressHelper;
    // addressHelper.SetBase("192.168.0.0", "255.255.255.0");

    NetDeviceContainer linkedDevices;
    Ipv4InterfaceContainer interfaces;
    Ipv4InterfaceContainer clientInterfaces;
    Ipv4InterfaceContainer routerInterfaces;
    Ipv4InterfaceContainer serverInterfaces;

    addressHelper.SetBase("192.168.0.0", "255.255.255.0");
    linkedDevices = PointToPointCreate(B, C, DataRate(g_link_a_BW), Time(g_link_a_delay), packetSize);
    interfaces = addressHelper.Assign(linkedDevices);
    serverInterfaces.Add(interfaces.Get(0));
    routerInterfaces.Add(interfaces.Get(1));

    addressHelper.SetBase("192.168.1.0", "255.255.255.0");
    // linkedDevices = PointToPointCreate(C, D, DataRate(g_link_c_BW), Time(g_link_b_delay), packetSize, g_router_b_buffer_size);
    linkedDevices = PointToPointCreate(C, D, DataRate(g_link_b_BW), Time(g_link_b_delay), packetSize, g_router_b_buffer_size);
    interfaces = addressHelper.Assign(linkedDevices);
    routerInterfaces.Add(interfaces.Get(0));
    routerInterfaces.Add(interfaces.Get(1));
    traceQueueDevices.Add(linkedDevices.Get(1));

    addressHelper.SetBase("192.168.9.0", "255.255.255.0");
    linkedDevices = PointToPointCreate(D, A, DataRate(g_link_b_BW), Time(g_link_b_delay), packetSize, g_router_b_buffer_size);
    interfaces = addressHelper.Assign(linkedDevices);
    routerInterfaces.Add(interfaces.Get(0));
    clientInterfaces.Add(interfaces.Get(1));
    // unstableDevices.push_back(linkedDevices.Get(0));
    // unstableDevices.push_back(linkedDevices.Get(1));

    if(g_link_b_BER != 0){
      std::cout << "Error model installed in link D-A" << std::endl;
      Ptr<RateErrorModel> ptr_em = CreateObjectWithAttributes<RateErrorModel> ();
      ptr_em->SetRate(g_link_b_BER);
      linkedDevices.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue (ptr_em));
      linkedDevices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue (ptr_em));
    }

    addressHelper.SetBase("192.168.2.0", "255.255.255.0");
    linkedDevices = PointToPointCreate(C, E, DataRate(g_link_c_BW), Time(g_link_c_delay), packetSize, g_router_c_buffer_size);
    addressHelper.Assign(linkedDevices);
    interfaces = addressHelper.Assign(linkedDevices);
    routerInterfaces.Add(interfaces.Get(0));
    routerInterfaces.Add(interfaces.Get(1));
    traceQueueDevices.Add(linkedDevices.Get(1));

    addressHelper.SetBase("192.168.11.0", "255.255.255.0");
    // linkedDevices = PointToPointCreate(E, A, DataRate(g_link_b_BW), Time(g_link_c_delay), packetSize, g_router_c_buffer_size);
    linkedDevices = PointToPointCreate(E, A, DataRate(g_link_c_BW), Time(g_link_c_delay), packetSize, g_router_c_buffer_size);
    interfaces = addressHelper.Assign(linkedDevices);
    routerInterfaces.Add(interfaces.Get(0));
    clientInterfaces.Add(interfaces.Get(1));
    // unstableDevices.push_back(linkedDevices.Get(0));
    // unstableDevices.push_back(linkedDevices.Get(1));

    //void ns3::Ipv4StaticRouting::AddHostRouteTo	(Ipv4Address dest, Ipv4Address nextHop, uint32_t interface, uint32_t metric = 0)
    // Notice that the 0th interface is bound to 127.0.0.0, and interface outgoing is from 1st.
    std::cout << "Hong jiaming 58: clientInterfaces number == " << clientInterfaces.GetN() << std::endl;
    Ptr<Ipv4StaticRouting> routing;
    routing = GetNodeStaticRoutingProtocol(A); // client
    routing->AddHostRouteTo(serverInterfaces.GetAddress(0), routerInterfaces.GetAddress(3), 1);
    routing->AddHostRouteTo(serverInterfaces.GetAddress(0), routerInterfaces.GetAddress(6), 2);

    routing = GetNodeStaticRoutingProtocol(B); // server
    routing->AddHostRouteTo(clientInterfaces.GetAddress(0), routerInterfaces.GetAddress(0), 1);
    routing->AddHostRouteTo(clientInterfaces.GetAddress(1), routerInterfaces.GetAddress(0), 1);

    routing = GetNodeStaticRoutingProtocol(C); // router
    routing->AddHostRouteTo(serverInterfaces.GetAddress(0), serverInterfaces.GetAddress(0), 1);
    routing->AddHostRouteTo(clientInterfaces.GetAddress(0), routerInterfaces.GetAddress(2), 2);
    routing->AddHostRouteTo(clientInterfaces.GetAddress(1), routerInterfaces.GetAddress(5), 3);

    routing = GetNodeStaticRoutingProtocol(D); // router
    routing->AddHostRouteTo(serverInterfaces.GetAddress(0), routerInterfaces.GetAddress(1), 1);
    routing->AddHostRouteTo(clientInterfaces.GetAddress(0), clientInterfaces.GetAddress(0), 2);
    // routing->AddHostRouteTo(clientInterfaces.GetAddress(0), clientInterfaces.GetAddress(0), 2);

    routing = GetNodeStaticRoutingProtocol(E); // router
    routing->AddHostRouteTo(serverInterfaces.GetAddress(0), routerInterfaces.GetAddress(4), 1);
    routing->AddHostRouteTo(clientInterfaces.GetAddress(1), clientInterfaces.GetAddress(1), 2);
  }

  return unstableDevices;
}



void CreateSimplestNetworkWithOtherTraffic (uint32_t packetSize,
                        NodeContainer& server,
                        NodeContainer& client,
                        NodeContainer& middle,
                        NodeContainer& other_servers,
                        NodeContainer& other_clients)
{
  //                   E(s)         D(c)
  //                   \    ----   /
  //                    \ /      \/
  // (1, server) B ---  C        A (0, client)
  //                     \      /
  //                       ----

  // #include "ns3/random-variable-stream.h"
  // Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();

  //Create the internet stack helper.
  client.Create(1);           // A, Alice

  server.Create(1);           // B, Bob

  middle.Create(1);           // C

  other_clients.Create(1);    // E

  other_servers.Create(1);    // D

  Ptr<Node> A = client.Get(0);
  Ptr<Node> B = server.Get(0);
  Ptr<Node> C = middle.Get(0);
  Ptr<Node> D = other_clients.Get(0);
  Ptr<Node> E = other_servers.Get(0);

  AnimationInterface::SetConstantPosition	(B, 0, 200);
  AnimationInterface::SetConstantPosition	(C, 200, 200);
  AnimationInterface::SetConstantPosition	(A, 400, 200);
  AnimationInterface::SetConstantPosition	(E, 100, 0);
  AnimationInterface::SetConstantPosition	(D, 500, 0);


  bool useStaticRouting = true;
  if(useStaticRouting == false){
    InternetStackHelper stackHelper = GetInternetStackHelper(useStaticRouting); // default value is false
    stackHelper.Install(client);
    stackHelper.Install(server);
    stackHelper.Install(middle);
    stackHelper.Install(other_clients);
    stackHelper.Install(other_servers);

    //Create the address helper
    Ipv4AddressHelper addressHelper;
    // addressHelper.SetBase("192.168.0.0", "255.255.255.0");

    addressHelper.SetBase("192.168.0.0", "255.255.255.0");
    addressHelper.Assign(PointToPointCreate(B, C, DataRate("300Kbps"), Time("6ms"), packetSize));

    addressHelper.SetBase("192.168.9.0", "255.255.255.0");
    addressHelper.Assign(PointToPointCreate(C, A, DataRate("100Kbps"), Time("500ms"), packetSize));

    addressHelper.SetBase("192.168.11.0", "255.255.255.0");
    addressHelper.Assign(PointToPointCreate(C, A, DataRate("100Kbps"), Time("15ms"), packetSize));

    addressHelper.SetBase("192.168.12.0", "255.255.255.0");
    addressHelper.Assign(PointToPointCreate(E, C, DataRate("100Kbps"), Time("15ms"), packetSize));

    addressHelper.SetBase("192.168.13.0", "255.255.255.0");
    addressHelper.Assign(PointToPointCreate(D, A, DataRate("100Kbps"), Time("15ms"), packetSize));
  }
  else{
    InternetStackHelper stackHelper = GetInternetStackHelper(useStaticRouting);
    stackHelper.Install(client);
    stackHelper.Install(server);
    stackHelper.Install(middle);
    stackHelper.Install(other_clients);
    stackHelper.Install(other_servers);

    //Create the address helper
    Ipv4AddressHelper addressHelper;
    // addressHelper.SetBase("192.168.0.0", "255.255.255.0");

    NetDeviceContainer linkedDevices;
    Ipv4InterfaceContainer interfaces;
    Ipv4InterfaceContainer clientInterfaces;
    Ipv4InterfaceContainer routerInterfaces;
    Ipv4InterfaceContainer serverInterfaces;
    Ipv4InterfaceContainer other_clientsInterfaces;
    Ipv4InterfaceContainer other_serversInterfaces;

    addressHelper.SetBase("192.168.0.0", "255.255.255.0");
    linkedDevices = PointToPointCreate(B, C, DataRate("300Kbps"), Time("6ms"), packetSize);
    interfaces = addressHelper.Assign(linkedDevices);
    serverInterfaces.Add(interfaces.Get(0));
    routerInterfaces.Add(interfaces.Get(1));

    addressHelper.SetBase("192.168.9.0", "255.255.255.0");
    linkedDevices = PointToPointCreate(C, A, DataRate("300Kbps"), Time("15ms"), packetSize);
    interfaces = addressHelper.Assign(linkedDevices);
    routerInterfaces.Add(interfaces.Get(0));
    clientInterfaces.Add(interfaces.Get(1));

    addressHelper.SetBase("192.168.11.0", "255.255.255.0");
    linkedDevices = PointToPointCreate(C, A, DataRate("100Kbps"), Time("50ms"), packetSize);
    addressHelper.Assign(linkedDevices);
    interfaces = addressHelper.Assign(linkedDevices);
    routerInterfaces.Add(interfaces.Get(0));
    clientInterfaces.Add(interfaces.Get(1));

    addressHelper.SetBase("192.168.12.0", "255.255.255.0");
    linkedDevices = PointToPointCreate(E, C, DataRate("100Kbps"), Time("50ms"), packetSize);
    addressHelper.Assign(linkedDevices);
    interfaces = addressHelper.Assign(linkedDevices);
    other_serversInterfaces.Add(interfaces.Get(0));
    routerInterfaces.Add(interfaces.Get(1));

    addressHelper.SetBase("192.168.13.0", "255.255.255.0");
    linkedDevices = PointToPointCreate(D, A, DataRate("100Kbps"), Time("50ms"), packetSize);
    addressHelper.Assign(linkedDevices);
    interfaces = addressHelper.Assign(linkedDevices);
    other_clientsInterfaces.Add(interfaces.Get(0));
    clientInterfaces.Add(interfaces.Get(1));

    // Ptr<RateErrorModel> ptr_em = CreateObjectWithAttributes<RateErrorModel> ();
    // ptr_em->SetRate(4*1e-4);
    // // d0_client0_Japan.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue (ptr_em));
    // linkedDevices.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue (ptr_em));
    // linkedDevices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue (ptr_em));


    //void ns3::Ipv4StaticRouting::AddHostRouteTo	(Ipv4Address dest, Ipv4Address nextHop, uint32_t interface, uint32_t metric = 0)
    // Notice that the 0th interface is bound to 127.0.0.0, and interface outgoing is from 1st.
    std::cout << "Hong jiaming 58: clientInterfaces == " << clientInterfaces.GetN() << std::endl;
    Ptr<Ipv4StaticRouting> routing;
    routing = GetNodeStaticRoutingProtocol(A); // client
    routing->AddHostRouteTo(serverInterfaces.GetAddress(0), routerInterfaces.GetAddress(1), 1);
    routing->AddHostRouteTo(serverInterfaces.GetAddress(0), routerInterfaces.GetAddress(2), 2);
    routing->AddHostRouteTo(other_clientsInterfaces.GetAddress(0), other_clientsInterfaces.GetAddress(0), 3);
    routing->AddHostRouteTo(other_serversInterfaces.GetAddress(0), routerInterfaces.GetAddress(1), 2);

    routing = GetNodeStaticRoutingProtocol(B); // server
    routing->AddHostRouteTo(clientInterfaces.GetAddress(0), routerInterfaces.GetAddress(0), 1);
    routing->AddHostRouteTo(clientInterfaces.GetAddress(1), routerInterfaces.GetAddress(0), 1);

    routing = GetNodeStaticRoutingProtocol(C); // router
    routing->AddHostRouteTo(serverInterfaces.GetAddress(0), serverInterfaces.GetAddress(0), 1);
    routing->AddHostRouteTo(clientInterfaces.GetAddress(0), clientInterfaces.GetAddress(0), 2);
    routing->AddHostRouteTo(clientInterfaces.GetAddress(1), clientInterfaces.GetAddress(1), 3);
    routing->AddHostRouteTo(other_clientsInterfaces.GetAddress(0), clientInterfaces.GetAddress(0), 2); // other traffic only use the upper path (9.1 to 9.2)
    routing->AddHostRouteTo(other_serversInterfaces.GetAddress(0), other_serversInterfaces.GetAddress(0), 4);

    routing = GetNodeStaticRoutingProtocol(D); // other_clients
    routing->AddHostRouteTo(other_serversInterfaces.GetAddress(0), clientInterfaces.GetAddress(2), 1);

    routing = GetNodeStaticRoutingProtocol(E); // other_servers
    routing->AddHostRouteTo(other_clientsInterfaces.GetAddress(0), routerInterfaces.GetAddress(3), 1);
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
  bool useStaticRouting = false;
  InternetStackHelper stackHelper = GetInternetStackHelper(useStaticRouting);

  //                        D ---- E
  //                      /   \  /  \
  // (1, server) B ---  C      /     A (0, client)
  //                     \   /  \   /
  //                       F ---- G


  client.Create(1);           // A
  stackHelper.Install(client);

  server.Create(1);           // D
  stackHelper.Install(server);

  middle.Create(5);             // B, C, E, F
  stackHelper.Install(middle);

  Ptr<Node> A = client.Get(0);
  Ptr<Node> B = server.Get(0);
  Ptr<Node> C = middle.Get(0);
  Ptr<Node> D = middle.Get(1);
  Ptr<Node> E = middle.Get(2);
  Ptr<Node> F = middle.Get(3);
  Ptr<Node> G = middle.Get(3);

  // other_servers.Add(B);
  // other_servers.Add(E);
  // other_clients.Add(F);
  // other_clients.Add(C);

  AnimationInterface::SetConstantPosition	(A, 800, 200);
  AnimationInterface::SetConstantPosition	(B, 0, 200);
  AnimationInterface::SetConstantPosition	(C, 200, 200);
  AnimationInterface::SetConstantPosition	(D, 400, 0);
  AnimationInterface::SetConstantPosition	(E, 600, 0);
  AnimationInterface::SetConstantPosition	(F, 400, 400);
  AnimationInterface::SetConstantPosition	(F, 600, 400);
  /*--------------------------------------*/

  //Create the address helper
  Ipv4AddressHelper addressHelper;
  // addressHelper.SetBase("192.168.0.0", "255.255.255.0");

  NetDeviceContainer linkedDevices;
  Ipv4InterfaceContainer interfaces;
  Ipv4InterfaceContainer clientInterfaces;
  Ipv4InterfaceContainer routerInterfaces;
  Ipv4InterfaceContainer serverInterfaces;

  addressHelper.SetBase("192.168.0.0", "255.255.255.0");
  linkedDevices = PointToPointCreate(B, C, DataRate("300Kbps"), Time("20ms"), packetSize);
  interfaces = addressHelper.Assign(linkedDevices);
  serverInterfaces.Add(interfaces.Get(0));
  routerInterfaces.Add(interfaces.Get(1));

  addressHelper.SetBase("192.168.1.0", "255.255.255.0");
  linkedDevices = PointToPointCreate(C, D, DataRate("200Kbps"), Time("2ms"), packetSize);
  interfaces = addressHelper.Assign(linkedDevices);
  routerInterfaces.Add(interfaces.Get(0));
  routerInterfaces.Add(interfaces.Get(1));

  addressHelper.SetBase("192.168.2.0", "255.255.255.0");
  linkedDevices = PointToPointCreate(C, F, DataRate("150Kbps"), Time("5ms"), packetSize);
  interfaces = addressHelper.Assign(linkedDevices);
  routerInterfaces.Add(interfaces.Get(0));
  routerInterfaces.Add(interfaces.Get(1));

  addressHelper.SetBase("192.168.3.0", "255.255.255.0");
  linkedDevices = PointToPointCreate(D, E, DataRate("100Kbps"), Time("5ms"), packetSize);
  interfaces = addressHelper.Assign(linkedDevices);
  routerInterfaces.Add(interfaces.Get(0));
  routerInterfaces.Add(interfaces.Get(1));

  addressHelper.SetBase("192.168.4.0", "255.255.255.0");
  linkedDevices = PointToPointCreate(D, G, DataRate("100Kbps"), Time("5ms"), packetSize);
  interfaces = addressHelper.Assign(linkedDevices);
  routerInterfaces.Add(interfaces.Get(0));
  routerInterfaces.Add(interfaces.Get(1));

  addressHelper.SetBase("192.168.5.0", "255.255.255.0");
  linkedDevices = PointToPointCreate(F, E, DataRate("50Kbps"), Time("5ms"), packetSize);
  interfaces = addressHelper.Assign(linkedDevices);
  routerInterfaces.Add(interfaces.Get(0));
  routerInterfaces.Add(interfaces.Get(1));

  addressHelper.SetBase("192.168.6.0", "255.255.255.0");
  linkedDevices = PointToPointCreate(F, G, DataRate("50Kbps"), Time("5ms"), packetSize);
  interfaces = addressHelper.Assign(linkedDevices);
  routerInterfaces.Add(interfaces.Get(0));
  routerInterfaces.Add(interfaces.Get(1));

  addressHelper.SetBase("192.168.9.0", "255.255.255.0");
  NetDeviceContainer d_EA = PointToPointCreate(E, A, DataRate("200Kbps"), Time("4ms"), packetSize);
  interfaces = addressHelper.Assign(d_EA);
  routerInterfaces.Add(interfaces.Get(0));
  clientInterfaces.Add(interfaces.Get(1));

  addressHelper.SetBase("192.168.11.0", "255.255.255.0");
  NetDeviceContainer d_GA = PointToPointCreate(G, A, DataRate("150Kbps"), Time("8ms"), packetSize);
  interfaces = addressHelper.Assign(d_GA);
  routerInterfaces.Add(interfaces.Get(0));
  clientInterfaces.Add(interfaces.Get(1));
  // Ptr<RateErrorModel> ptr_em = CreateObjectWithAttributes<RateErrorModel> ();
  // ptr_em->SetRate(2*1e-5);
  // // d0_client0_Japan.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue (ptr_em));
  // d_CD.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue (ptr_em));
  // d_CD.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue (ptr_em));

  std::cout << "Hong jiaming 58: clientInterfaces == " << clientInterfaces.GetN() << std::endl;


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

void ChangeLinkErrorRate (Ptr<NetDevice> nd, double bitErrorRate){
    Ptr<RateErrorModel> error = CreateObject<RateErrorModel> ();
    error->SetAttribute ("ErrorRate", DoubleValue (bitErrorRate));
    nd->SetAttribute ("ReceiveErrorModel", PointerValue (error));
}

};
