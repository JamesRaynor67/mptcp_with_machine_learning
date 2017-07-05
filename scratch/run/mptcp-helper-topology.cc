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
  // tchRed.SetRootQueueDisc ("ns3::RedQueueDisc",
  //                         "MeanPktSize", UintegerValue(packetSize),
  //                         "LinkBandwidth", DataRateValue(linkRate),
  //                         "LinkDelay", TimeValue(delay));
  tchRed.SetRootQueueDisc ("ns3::PfifoFastQueueDisc", "Limit", UintegerValue(queueSize));
  // tchRed.SetAttribute("Limit", UintegerValue(1));

  NS_ASSERT(queueSize > 0);
  pointToPoint.SetQueue("ns3::DropTailQueue",
                        "MaxPackets", UintegerValue(queueSize));
  pointToPoint.EnablePcapAll ("mptcp"); // This is only for debug use
  NetDeviceContainer linkedDevices;
  linkedDevices = pointToPoint.Install (linkedNodes);

  tchRed.Install(linkedDevices);

  return linkedDevices;
}

void CreateSimplestNetwork (uint32_t packetSize,
                        NodeContainer& server,
                        NodeContainer& client,
                        NodeContainer& middle,
                        NodeContainer& other_servers,
                        NodeContainer& other_clients,
                        NetDeviceContainer& traceQueueDevices,                        
                        NetDeviceContainer& unstableDevices)
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
    // unstableDevices.Add(linkedDevices.Get(0));
    // unstableDevices.Add(linkedDevices.Get(1));

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
    // unstableDevices.Add(linkedDevices.Get(0));
    // unstableDevices.Add(linkedDevices.Get(1));

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
}

void CreateNetwork5 (uint32_t packetSize,
                        NodeContainer& server,
                        NodeContainer& client,
                        NodeContainer& middle,
                        NodeContainer& other_servers,
                        NodeContainer& other_clients,
                        NetDeviceContainer& traceQueueDevices,
                        NetDeviceContainer& unstableDevices)
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
    // unstableDevices.Add(linkedDevices.Get(0));
    // unstableDevices.Add(linkedDevices.Get(1));

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
    // unstableDevices.Add(linkedDevices.Get(0));
    // unstableDevices.Add(linkedDevices.Get(1));

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
}

void CreateNetwork11 (uint32_t packetSize,
                        NodeContainer& server,
                        NodeContainer& client,
                        NodeContainer& middle,
                        NodeContainer& other_servers,
                        NodeContainer& other_clients,
                        NetDeviceContainer& traceQueueDevices,
                        NetDeviceContainer& unstableDevices)
{
  //                     H (s0)    J (c0)
  //                      \       /
  //                       D --- E
  //                      /       \
  //  (1, server)  B --- C         A (0, client)
  //                      \       /
  //                       F --- G
  //                      /       \
  //                     I (s1)    K (c1)

  //Create the internet stack helper.
  client.Create(1);           // A, Alice
  server.Create(1);           // B, Bob
  middle.Create(5);           // C, D, E, F, G
  other_servers.Create(2);     // H, I
  other_clients.Create(2);     // J, K

  Ptr<Node> A = client.Get(0);
  Ptr<Node> B = server.Get(0);
  Ptr<Node> C = middle.Get(0);
  Ptr<Node> D = middle.Get(1);
  Ptr<Node> E = middle.Get(2);
  Ptr<Node> F = middle.Get(3);
  Ptr<Node> G = middle.Get(4);
  Ptr<Node> H = other_servers.Get(0);
  Ptr<Node> I = other_servers.Get(1);
  Ptr<Node> J = other_clients.Get(0);
  Ptr<Node> K = other_clients.Get(1);

  AnimationInterface::SetConstantPosition	(B, 0, 200);
  AnimationInterface::SetConstantPosition	(C, 200, 200);
  AnimationInterface::SetConstantPosition	(A, 600, 200);
  AnimationInterface::SetConstantPosition	(D, 300, 100);
  AnimationInterface::SetConstantPosition	(E, 500, 100);
  AnimationInterface::SetConstantPosition	(F, 300, 300);
  AnimationInterface::SetConstantPosition	(G, 500, 300);
  AnimationInterface::SetConstantPosition	(H, 200, 0);
  AnimationInterface::SetConstantPosition	(J, 600, 0);
  AnimationInterface::SetConstantPosition	(I, 200, 400);
  AnimationInterface::SetConstantPosition	(K, 600, 400);

  bool useStaticRouting = true;
  if(useStaticRouting == false){
    InternetStackHelper stackHelper = GetInternetStackHelper(useStaticRouting);
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

    addressHelper.SetBase("192.168.1.0", "255.255.255.0");
    addressHelper.Assign(PointToPointCreate(C, D, DataRate("300Kbps"), Time("6ms"), packetSize));

    addressHelper.SetBase("192.168.2.0", "255.255.255.0");
    addressHelper.Assign(PointToPointCreate(C, F, DataRate("100Kbps"), Time("500ms"), packetSize));

    addressHelper.SetBase("192.168.3.0", "255.255.255.0");
    addressHelper.Assign(PointToPointCreate(D, H, DataRate("100Kbps"), Time("15ms"), packetSize));

    addressHelper.SetBase("192.168.4.0", "255.255.255.0");
    addressHelper.Assign(PointToPointCreate(D, E, DataRate("300Kbps"), Time("6ms"), packetSize));

    addressHelper.SetBase("192.168.5.0", "255.255.255.0");
    addressHelper.Assign(PointToPointCreate(E, J, DataRate("100Kbps"), Time("500ms"), packetSize));

    addressHelper.SetBase("192.168.6.0", "255.255.255.0");
    addressHelper.Assign(PointToPointCreate(F, I, DataRate("100Kbps"), Time("15ms"), packetSize));

    addressHelper.SetBase("192.168.7.0", "255.255.255.0");
    addressHelper.Assign(PointToPointCreate(F, G, DataRate("300Kbps"), Time("6ms"), packetSize));

    addressHelper.SetBase("192.168.8.0", "255.255.255.0");
    addressHelper.Assign(PointToPointCreate(G, K, DataRate("100Kbps"), Time("500ms"), packetSize));

    addressHelper.SetBase("192.168.9.0", "255.255.255.0");
    addressHelper.Assign(PointToPointCreate(E, A, DataRate("100Kbps"), Time("500ms"), packetSize));

    addressHelper.SetBase("192.168.11.0", "255.255.255.0");
    addressHelper.Assign(PointToPointCreate(G, A, DataRate("100Kbps"), Time("15ms"), packetSize));    
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
    Ipv4InterfaceContainer otherClientsInterfaces;
    Ipv4InterfaceContainer otherServersInterfaces;

    addressHelper.SetBase("192.168.0.0", "255.255.255.0");
    linkedDevices = PointToPointCreate(B, C, DataRate(g_link_a_BW), Time(g_link_a_delay), packetSize);
    Ipv4InterfaceContainer bcInterfaces = addressHelper.Assign(linkedDevices);

    addressHelper.SetBase("192.168.1.0", "255.255.255.0");
    linkedDevices = PointToPointCreate(C, D, DataRate(g_link_b_BW), Time(g_link_b_delay), packetSize, g_router_b_buffer_size);
    Ipv4InterfaceContainer cdInterfaces = addressHelper.Assign(linkedDevices);

    addressHelper.SetBase("192.168.2.0", "255.255.255.0");
    linkedDevices = PointToPointCreate(C, F, DataRate(g_link_c_BW), Time(g_link_c_delay), packetSize, g_router_c_buffer_size);
    Ipv4InterfaceContainer cfInterfaces = addressHelper.Assign(linkedDevices);

    addressHelper.SetBase("192.168.3.0", "255.255.255.0");
    linkedDevices = PointToPointCreate(D, H, DataRate(g_link_c_BW), Time(g_link_c_delay), packetSize, g_router_c_buffer_size); // 
    Ipv4InterfaceContainer dhInterfaces = addressHelper.Assign(linkedDevices);

    addressHelper.SetBase("192.168.4.0", "255.255.255.0");
    linkedDevices = PointToPointCreate(D, E, DataRate(g_link_b_BW), Time(g_link_b_delay), packetSize, g_router_b_buffer_size);
    Ipv4InterfaceContainer deInterfaces = addressHelper.Assign(linkedDevices);

    addressHelper.SetBase("192.168.5.0", "255.255.255.0");
    linkedDevices = PointToPointCreate(E, J, DataRate(g_link_c_BW), Time(g_link_c_delay), packetSize, g_router_c_buffer_size); //
    Ipv4InterfaceContainer eiInterfaces = addressHelper.Assign(linkedDevices);

    addressHelper.SetBase("192.168.6.0", "255.255.255.0");
    linkedDevices = PointToPointCreate(F, I, DataRate(g_link_c_BW), Time(g_link_c_delay), packetSize, g_router_c_buffer_size); //
    Ipv4InterfaceContainer fjInterfaces = addressHelper.Assign(linkedDevices);

    addressHelper.SetBase("192.168.7.0", "255.255.255.0");
    linkedDevices = PointToPointCreate(F, G, DataRate(g_link_c_BW), Time(g_link_c_delay), packetSize, g_router_c_buffer_size);
    Ipv4InterfaceContainer fgInterfaces = addressHelper.Assign(linkedDevices);

    addressHelper.SetBase("192.168.8.0", "255.255.255.0");
    linkedDevices = PointToPointCreate(G, K, DataRate(g_link_c_BW), Time(g_link_c_delay), packetSize, g_router_c_buffer_size); //
    Ipv4InterfaceContainer gkInterfaces = addressHelper.Assign(linkedDevices);

    addressHelper.SetBase("192.168.9.0", "255.255.255.0");
    linkedDevices = PointToPointCreate(E, A, DataRate(g_link_b_BW), Time(g_link_b_delay), packetSize, g_router_b_buffer_size); //
    Ipv4InterfaceContainer eaInterfaces = addressHelper.Assign(linkedDevices);
    traceQueueDevices.Add(linkedDevices.Get(0));  // Trace queue length in Node E
    if(g_link_b_BER != 0){                        // Although g_link_b_BER is double, it's still OK to compare it with 0
      std::cout << "Error model installed in link E-A" << std::endl;
      Ptr<RateErrorModel> ptr_em = CreateObjectWithAttributes<RateErrorModel> ();
      ptr_em->SetRate(g_link_b_BER);
      linkedDevices.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue (ptr_em));
      linkedDevices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue (ptr_em));
    }
    // unstableDevices.push_back(linkedDevices.Get(0));
    // unstableDevices.push_back(linkedDevices.Get(1));

    addressHelper.SetBase("192.168.11.0", "255.255.255.0");
    linkedDevices = PointToPointCreate(G, A, DataRate(g_link_c_BW), Time(g_link_c_delay), packetSize, g_router_c_buffer_size);
    Ipv4InterfaceContainer gaInterfaces = addressHelper.Assign(linkedDevices);
    traceQueueDevices.Add(linkedDevices.Get(0)); // Trace queue length in Node G
    
    Ptr<Ipv4StaticRouting> routing;
    routing = GetNodeStaticRoutingProtocol(A); // client
    routing->AddHostRouteTo(bcInterfaces.GetAddress(0), eaInterfaces.GetAddress(0), 1);
    routing->AddHostRouteTo(bcInterfaces.GetAddress(0), gaInterfaces.GetAddress(0), 2);

    routing = GetNodeStaticRoutingProtocol(B); // server
    routing->AddHostRouteTo(eaInterfaces.GetAddress(1), bcInterfaces.GetAddress(1), 1);
    routing->AddHostRouteTo(gaInterfaces.GetAddress(1), bcInterfaces.GetAddress(1), 1);

    routing = GetNodeStaticRoutingProtocol(C); // router
    routing->AddHostRouteTo(bcInterfaces.GetAddress(0), bcInterfaces.GetAddress(0), 1);
    routing->AddHostRouteTo(eaInterfaces.GetAddress(1), cdInterfaces.GetAddress(1), 2);
    routing->AddHostRouteTo(gaInterfaces.GetAddress(1), cfInterfaces.GetAddress(1), 3);

    routing = GetNodeStaticRoutingProtocol(D); // router
    routing->AddHostRouteTo(bcInterfaces.GetAddress(0), cdInterfaces.GetAddress(0), 1);
    routing->AddHostRouteTo(dhInterfaces.GetAddress(1), dhInterfaces.GetAddress(1), 2);
    routing->AddHostRouteTo(eaInterfaces.GetAddress(1), deInterfaces.GetAddress(1), 3);
    routing->AddHostRouteTo(eiInterfaces.GetAddress(1), deInterfaces.GetAddress(1), 3);

    routing = GetNodeStaticRoutingProtocol(E); // router
    routing->AddHostRouteTo(bcInterfaces.GetAddress(0), deInterfaces.GetAddress(0), 1);
    routing->AddHostRouteTo(dhInterfaces.GetAddress(1), deInterfaces.GetAddress(0), 1);
    routing->AddHostRouteTo(eiInterfaces.GetAddress(1), eiInterfaces.GetAddress(1), 2);
    routing->AddHostRouteTo(eaInterfaces.GetAddress(1), eaInterfaces.GetAddress(1), 3);

    routing = GetNodeStaticRoutingProtocol(F); // router
    routing->AddHostRouteTo(bcInterfaces.GetAddress(0), cfInterfaces.GetAddress(0), 1);
    routing->AddHostRouteTo(fjInterfaces.GetAddress(1), fjInterfaces.GetAddress(1), 2);
    routing->AddHostRouteTo(gkInterfaces.GetAddress(1), fgInterfaces.GetAddress(1), 3);
    routing->AddHostRouteTo(gaInterfaces.GetAddress(1), fgInterfaces.GetAddress(1), 3);

    routing = GetNodeStaticRoutingProtocol(G); // router
    routing->AddHostRouteTo(bcInterfaces.GetAddress(0), fgInterfaces.GetAddress(0), 1);
    routing->AddHostRouteTo(fjInterfaces.GetAddress(1), fgInterfaces.GetAddress(0), 1);
    routing->AddHostRouteTo(gkInterfaces.GetAddress(1), gkInterfaces.GetAddress(1), 2);
    routing->AddHostRouteTo(gaInterfaces.GetAddress(1), gaInterfaces.GetAddress(1), 3);

    routing = GetNodeStaticRoutingProtocol(H); // other server
    routing->AddHostRouteTo(eiInterfaces.GetAddress(1), dhInterfaces.GetAddress(0), 1);

    routing = GetNodeStaticRoutingProtocol(J); // other client
    routing->AddHostRouteTo(dhInterfaces.GetAddress(1), eiInterfaces.GetAddress(0), 1);

    routing = GetNodeStaticRoutingProtocol(I); // other server
    routing->AddHostRouteTo(gkInterfaces.GetAddress(1), fjInterfaces.GetAddress(0), 1);

    routing = GetNodeStaticRoutingProtocol(K); // router
    routing->AddHostRouteTo(fjInterfaces.GetAddress(1), gkInterfaces.GetAddress(0), 1);
  }
}


void ChangeLinkErrorRate (Ptr<NetDevice> nd, double bitErrorRate){
    Ptr<RateErrorModel> error = CreateObject<RateErrorModel> ();
    error->SetAttribute ("ErrorRate", DoubleValue (bitErrorRate));
    nd->SetAttribute ("ReceiveErrorModel", PointerValue (error));
}

};