#include <string>
#include <utility>
#include <vector>

#include "mptcp-helper-system.h"
#include "mptcp-helper-router.h"
#include "mptcp-helper-trace.h"
#include "mptcp-helper-topology.h"
#include "mptcp-helper-application.h"
#include "ns3/rl-data-interface.h"

#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/global-route-manager.h"

using namespace ns3;

int main(int argc, char* argv[])
{
  enum AppType
  {
    onoff = 0,
    filetransfer = 1
  };

  uint32_t type = 0;
  uint32_t appType = 0;
  string outputDir = "mptcp_output";
  uint32_t simulationDuration = 120;

  CommandLine cmd;
  cmd.AddValue("outputDir", "The output directory to write the logs to.", outputDir);
  cmd.AddValue("appType", "The type of application to run", appType);
  cmd.Parse(argc, argv);

  string linkRate = "10Mbps";
  string linkDelay = "10ms";

  // Assume the maximum size of mptcp option is 40 bytes
  uint32_t tcpOptionSize = 40;
  uint32_t headersSize = 20 + 20 + 2 + tcpOptionSize; //ipheader + tcpheader + pppheader + tcp options
  uint32_t segmentSize = 1500;
  uint32_t segmentSizeWithoutHeaders = segmentSize - headersSize;

  DataRate rate(linkRate);
  Time delay(linkDelay);

  uint32_t bdp = rate.GetBitRate() * delay.GetSeconds() * 8; // bandwidth-delay product (Long Fat Network if bdp is significantly larger than 10^5 bits)
  uint32_t bdpBytes = bdp/8;
  uint32_t queueSize = 1.5 * max<uint32_t>(bdpBytes, 10 * segmentSize);

  //Enable logging
  EnableLogging ();

  SetConfigDefaults(linkRate, linkDelay, segmentSize, segmentSizeWithoutHeaders, queueSize);

  // //Create the nodes in the topology, and install the internet stack on them
  // NodeContainer server;
  // NodeContainer client;
  // NodeContainer isps;
  // NodeContainer ixs;    // Internet exchange points
  // NodeContainer other_servers;
  // NodeContainer other_clients;
  // CreateRealNetwork (segmentSizeWithoutHeaders, server, client, isps, ixs, other_servers, other_clients);


  NodeContainer server;
  NodeContainer client;
  NodeContainer middle;
  NodeContainer other_servers;
  NodeContainer other_clients;
  // CreateExtendedClassicNetwork (segmentSizeWithoutHeaders, server, client, middle, other_servers, other_clients);
  // CreateClassicNetworkWithOtherTraffic (segmentSizeWithoutHeaders, server, client, middle, other_servers, other_clients);
  CreateSimplestNetwork(segmentSizeWithoutHeaders, server, client, middle, other_servers, other_clients);
  // CreateSimplestNetworkWithOtherTraffic(segmentSizeWithoutHeaders, server, client, middle, other_servers, other_clients);
  // CreateClassicNetwork(segmentSizeWithoutHeaders, server, client, middle, other_servers, other_clients);
  ConfigureTracing(outputDir, server, client, middle, other_servers, other_clients);

  //Create and install the applications on the server and client
  if(appType == onoff)
  {
    std::cout << "Application type: onoff\n";
    // NodeContainer tmp_servers;
    // NodeContainer tmp_clients;
    // tmp_servers.Add(server);
    // tmp_servers.Add(other_servers);
    // tmp_clients.Add(client);
    // tmp_clients.Add(other_clients);
    // InstallOnOffApplications(server, client, segmentSizeWithoutHeaders);
    // InstallOnOffApplications(other_servers, other_clients, segmentSizeWithoutHeaders);
    // InstallOnOffApplications(tmp_servers, tmp_clients, segmentSizeWithoutHeaders);
    InstallFileTransferApplications(server, client, segmentSizeWithoutHeaders, queueSize);
    // InstallFileTransferApplications(other_servers, other_clients, segmentSizeWithoutHeaders, queueSize);
  }
  else if (appType == filetransfer)
  {
    std::cout << "Application type: filetransfer\n";
    InstallFileTransferApplications(server, client, segmentSizeWithoutHeaders, queueSize);
  }

  //Populate and print the IP routing tables
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  Ipv4GlobalRoutingHelper g;
  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("dynamic-global-routing-mptcp.routes", std::ios::out);
  g.PrintRoutingTableAllAt (Seconds (0), routingStream);
  Ptr<OutputStreamWrapper> routingStream2 = Create<OutputStreamWrapper> ("dynamic-global-routing-mptcp.routes-16", std::ios::out);
  g.PrintRoutingTableAllAt (Seconds (16), routingStream2);
  //Create an output directory and configure tracing
  AnimationInterface anim ("mptcp-animation.xml");

  // Simulator::Schedule(Seconds(7.9), &PrintMonitorStates);
  // Simulator::Schedule(Seconds(8), &PrintMonitorStates);
  // Simulator::Schedule(Seconds(12), &PrintMonitorStates);
  // Simulator::Schedule(Seconds(16), &PrintMonitorStates);
  // Simulator::Schedule(Seconds(20), &PrintMonitorStates);
  // Simulator::Schedule(Seconds(30), &PrintMonitorStates);
  for(int i = 0; i < simulationDuration * 10;i++){
    Simulator::Schedule(Seconds(i/10.0), &TraceMonitorStates, outputDir);

  }
  Simulator::Stop (Seconds(simulationDuration));

  for(int i = 0;i < client.GetN();i++){
    std::cout << "\nclient: " << "node " << client.Get(i)->GetId() << "\n";
    Ptr<Node> node = client.Get (i); // Get pointer to ith node in container
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> (); // Get Ipv4 instance of the node
    for(int j = 0;j < ipv4->GetNInterfaces();j++){
      Ipv4Address addr = ipv4->GetAddress (j, 0).GetLocal (); // Get Ipv4InterfaceAddress of xth interface.
      std::cout  << "interface " << j << " " << ipv4->GetNetDevice(j) << " : " << addr << '\t';
    }
  }

  for(int i = 0;i < server.GetN();i++){
    std::cout << "\nserver: " << "node " << server.Get(i)->GetId() << "\n";
    Ptr<Node> node = server.Get (i); // Get pointer to ith node in container
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> (); // Get Ipv4 instance of the node
    for(int j = 0;j < ipv4->GetNInterfaces();j++){
      Ipv4Address addr = ipv4->GetAddress (j, 0).GetLocal (); // Get Ipv4InterfaceAddress of xth interface.
      std::cout << "interface " << j << " " << ipv4->GetNetDevice(j) << " : " << addr << '\t';
    }
  }

  for(int i = 0;i < middle.GetN();i++){
    std::cout << "\nmiddle: " << "node " << middle.Get(i)->GetId() << "\n";
    Ptr<Node> node = middle.Get (i); // Get pointer to ith node in container
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> (); // Get Ipv4 instance of the node
    for(int j = 0;j < ipv4->GetNInterfaces();j++){
      Ipv4Address addr = ipv4->GetAddress (j, 0).GetLocal (); // Get Ipv4InterfaceAddress of xth interface.
      std::cout << "interface " << j << " " << ipv4->GetNetDevice(j) << " : " << addr << '\t';
    }
  }

  for(int i = 0;i < other_clients.GetN();i++){
    std::cout << "\nother_clients: " << "node " << other_clients.Get(i)->GetId() << "\n";
    Ptr<Node> node = other_clients.Get (i); // Get pointer to ith node in container
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> (); // Get Ipv4 instance of the node
    for(int j = 0;j < ipv4->GetNInterfaces();j++){
      Ipv4Address addr = ipv4->GetAddress (j, 0).GetLocal (); // Get Ipv4InterfaceAddress of xth interface.
      std::cout << "interface " << j << " " << ipv4->GetNetDevice(j) << " : " << addr << '\t';
    }
  }

  for(int i = 0;i < other_servers.GetN();i++){
    std::cout << "\nother_servers: " << "node " << other_servers.Get(i)->GetId() << "\n";
    Ptr<Node> node = other_servers.Get (i); // Get pointer to ith node in container
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> (); // Get Ipv4 instance of the node
    for(int j = 0;j < ipv4->GetNInterfaces();j++){
      Ipv4Address addr = ipv4->GetAddress (j, 0).GetLocal (); // Get Ipv4InterfaceAddress of xth interface.
      std::cout << "interface " << j << " " << ipv4->GetNetDevice(j) << " : " << addr << '\t';
    }
  }
  std::cout << "\n\n";
  //Begin the simulation
  Simulator::Run ();

  Simulator::Destroy ();
  return 0;
}


//////////*****************///////////////////////////////
// #include "ns3/core-module.h"
// #include "ns3/network-module.h"
// #include "ns3/mobility-module.h"
// #include "ns3/config-store-module.h"
// #include "ns3/wifi-module.h"
// #include "ns3/internet-module.h"
//
// #include <iostream>
// #include <fstream>
// #include <vector>
// #include <string>
//
// using namespace ns3;
//
// NS_LOG_COMPONENT_DEFINE ("WifiSimpleAdhoc");
//
// void ReceivePacket (Ptr<Socket> socket)
// {
//   while (socket->Recv ())
//     {
//       NS_LOG_UNCOND ("Received one packet!");
//     }
// }
//
// static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize,
//                              uint32_t pktCount, Time pktInterval )
// {
//   if (pktCount > 0)
//     {
//       socket->Send (Create<Packet> (pktSize));
//       Simulator::Schedule (pktInterval, &GenerateTraffic, socket, pktSize, pktCount-1, pktInterval);
//     }
//   else
//     {
//       socket->Close ();
//     }
// }
//
//
// int main (int argc, char *argv[])
// {
//   std::string phyMode ("DsssRate1Mbps");
//   double rss = -80;  // -dBm
//   uint32_t packetSize = 1000; // bytes
//   uint32_t numPackets = 1;
//   double interval = 1.0; // seconds
//   bool verbose = false;
//
//   CommandLine cmd;
//
//   cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
//   cmd.AddValue ("rss", "received signal strength", rss);
//   cmd.AddValue ("packetSize", "size of application packet sent", packetSize);
//   cmd.AddValue ("numPackets", "number of packets generated", numPackets);
//   cmd.AddValue ("interval", "interval (seconds) between packets", interval);
//   cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);
//
//   cmd.Parse (argc, argv);
//   // Convert to time object
//   Time interPacketInterval = Seconds (interval);
//
//   // disable fragmentation for frames below 2200 bytes
//   Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
//   // turn off RTS/CTS for frames below 2200 bytes
//   Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
//   // Fix non-unicast data rate to be the same as that of unicast
//   Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));
//
//   NodeContainer c;
//   c.Create (2);
//
//   // The below set of helpers will help us to put together the wifi NICs we want
//   WifiHelper wifi;
//   if (verbose)
//     {
//       wifi.EnableLogComponents ();  // Turn on all Wifi logging
//     }
//   wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
//
//   YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
//   // This is one parameter that matters when using FixedRssLossModel
//   // set it to zero; otherwise, gain will be added
//   wifiPhy.Set ("RxGain", DoubleValue (0) );
//   // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
//   wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
//
//   YansWifiChannelHelper wifiChannel;
//   wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
//   // The below FixedRssLossModel will cause the rss to be fixed regardless
//   // of the distance between the two stations, and the transmit power
//   wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",DoubleValue (rss));
//   wifiPhy.SetChannel (wifiChannel.Create ());
//
//   // Add a mac and disable rate control
//   WifiMacHelper wifiMac;
//   wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
//                                 "DataMode",StringValue (phyMode),
//                                 "ControlMode",StringValue (phyMode));
//   // Set it to adhoc mode
//   wifiMac.SetType ("ns3::AdhocWifiMac");
//   NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, c);
//
//   // Note that with FixedRssLossModel, the positions below are not
//   // used for received signal strength.
//   MobilityHelper mobility;
//   Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
//   positionAlloc->Add (Vector (0.0, 0.0, 0.0));
//   positionAlloc->Add (Vector (5.0, 0.0, 0.0));
//   mobility.SetPositionAllocator (positionAlloc);
//   mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
//   mobility.Install (c);
//
//   InternetStackHelper internet;
//   internet.Install (c);
//
//   Ipv4AddressHelper ipv4;
//   NS_LOG_INFO ("Assign IP Addresses.");
//   ipv4.SetBase ("10.1.1.0", "255.255.255.0");
//   Ipv4InterfaceContainer i = ipv4.Assign (devices);
//
//   // TypeId tid = TypeId::LookupByName ("ns3::MpTcpSocketFactory");
//   // TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory"); // TCP or MPTCP is not available here
//   TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
//   Ptr<Socket> recvSink = Socket::CreateSocket (c.Get (0), tid);
//   InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
//   recvSink->Bind (local);
//   recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));
//
//   Ptr<Socket> source = Socket::CreateSocket (c.Get (1), tid);
//   InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 80);
//   source->SetAllowBroadcast (true);
//   source->Connect (remote);
//
//   // Tracing
//   wifiPhy.EnablePcap ("wifi-simple-adhoc", devices);
//
//   // Output what we are doing
//   NS_LOG_UNCOND ("Testing " << numPackets  << " packets sent with receiver rss " << rss );
//
//   Simulator::ScheduleWithContext (source->GetNode ()->GetId (),
//                                   Seconds (1.0), &GenerateTraffic,
//                                   source, packetSize, numPackets, interPacketInterval);
//
//   Simulator::Run ();
//   Simulator::Destroy ();
//
//   return 0;
// }





// #include <string>
// #include <utility>
// #include <vector>
//
// #include "mptcp-helper-system.h"
// #include "mptcp-helper-router.h"
// #include "mptcp-helper-trace.h"
// #include "mptcp-helper-topology.h"
// #include "mptcp-helper-application.h"
// #include "mp-onoff-application.h"
// #include "file-transfer-helper.h"
// #include "file-transfer-application.h"
//
// #include "ns3/rl-data-interface.h"
//
// #include "ns3/olsr-helper.h"
// #include "ns3/ipv4-static-routing-helper.h"
// #include "ns3/ipv4-list-routing-helper.h"
// #include "ns3/flow-monitor-module.h"
// #include "ns3/netanim-module.h"
// // #include "ns3/core-module.h"
// // #include "ns3/internet-module.h"
// // #include "ns3/point-to-point-module.h"
// // #include "ns3/applications-module.h"
// #include "ns3/traffic-control-module.h"
// #include "ns3/global-route-manager.h"
//
// #include "ns3/applications-module.h"
// #include "ns3/core-module.h"
// #include "ns3/internet-module.h"
// #include "ns3/mobility-module.h"
// #include "ns3/network-module.h"
// #include "ns3/point-to-point-module.h"
// #include "ns3/wifi-module.h"
//
// NS_LOG_COMPONENT_DEFINE ("wifi-tcp");
//
// using namespace ns3;
//
// Ptr<Ipv4StaticRouting> GetNodeStaticRoutingProtocol(Ptr<Node> node)
// {
//   //std::cout << "WTF 1" << std::endl;
//   Ptr<Ipv4> nodeIpv4 = node->GetObject<Ipv4> ();
//   Ptr<Ipv4RoutingProtocol> routingProtocol = nodeIpv4->GetRoutingProtocol();
//   Ptr<Ipv4ListRouting> listRouting = DynamicCast<Ipv4ListRouting>(routingProtocol);
//   int16_t priority;
//   Ptr<Ipv4StaticRouting> routing = DynamicCast<Ipv4StaticRouting>(listRouting->GetRoutingProtocol(0, priority));
//   //std::cout << "WTF 2" << std::endl;
//   return routing;
// }
//
// // Ptr<PacketSink> sink;                         /* Pointer to the packet sink application */
// // uint64_t lastTotalRx = 0;                     /* The value of the last total received bytes */
// //
// // void
// // CalculateThroughput ()
// // {
// //   Time now = Simulator::Now ();                                         /* Return the simulator's virtual time. */
// //   double cur = (sink->GetTotalRx() - lastTotalRx) * (double) 8/1e5;     /* Convert Application RX Packets to MBits. */
// //   std::cout << now.GetSeconds () << "s: \t" << cur << " Mbit/s" << std::endl;
// //   lastTotalRx = sink->GetTotalRx ();
// //   Simulator::Schedule (MilliSeconds (100), &CalculateThroughput);
// // }
//
// int
// main(int argc, char *argv[])
// {
//   uint32_t payloadSize = 1472;                       /* Transport layer payload size in bytes. */
//   std::string dataRate = "100Mbps";                  /* Application layer datarate. */
//   std::string tcpVariant = "ns3::TcpNewReno";        /* TCP variant type. */
//   std::string phyRate = "HtMcs7";                    /* Physical layer bitrate. */
//   double simulationTime = 2.1;                        /* Simulation time in seconds. */
//   bool pcapTracing = false;                          /* PCAP Tracing is enabled or not. */
//
//   /* Command line argument parser setup. */
//   CommandLine cmd;
//   cmd.AddValue ("payloadSize", "Payload size in bytes", payloadSize);
//   cmd.AddValue ("dataRate", "Application data ate", dataRate);
//   cmd.AddValue ("tcpVariant", "Transport protocol to use: TcpTahoe, TcpReno, TcpNewReno, TcpWestwood, TcpWestwoodPlus ", tcpVariant);
//   cmd.AddValue ("phyRate", "Physical layer bitrate", phyRate);
//   cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
//   cmd.AddValue ("pcap", "Enable/disable PCAP Tracing", pcapTracing);
//   cmd.Parse (argc, argv);
//
//   /* No fragmentation and no RTS/CTS */
//   Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("999999"));
//   Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("999999"));
//
//   /* Configure TCP Options */
//   Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize));
//
//   WifiMacHelper wifiMac;
//   WifiHelper wifiHelper;
//   wifiHelper.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
//
//   /* Set up Legacy Channel */
//   YansWifiChannelHelper wifiChannel ;
//   wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
//   wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency", DoubleValue (5e9));
//
//   /* Setup Physical Layer */
//   YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
//   wifiPhy.SetChannel (wifiChannel.Create ());
//   wifiPhy.Set ("TxPowerStart", DoubleValue (10.0));
//   wifiPhy.Set ("TxPowerEnd", DoubleValue (10.0));
//   wifiPhy.Set ("TxPowerLevels", UintegerValue (1));
//   wifiPhy.Set ("TxGain", DoubleValue (0));
//   wifiPhy.Set ("RxGain", DoubleValue (0));
//   wifiPhy.Set ("RxNoiseFigure", DoubleValue (10));
//   wifiPhy.Set ("CcaMode1Threshold", DoubleValue (-79));
//   wifiPhy.Set ("EnergyDetectionThreshold", DoubleValue (-79 + 3));
//   wifiPhy.SetErrorRateModel ("ns3::YansErrorRateModel");
//   wifiHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
//                                       "DataMode", StringValue (phyRate),
//                                       "ControlMode", StringValue ("HtMcs0"));
//
//   NodeContainer networkNodes;
//   networkNodes.Create (2);
//   Ptr<Node> apWifiNode = networkNodes.Get (0);
//   Ptr<Node> staWifiNode = networkNodes.Get (1);
//
//   /* Configure AP */
//   Ssid ssid = Ssid ("network");
//   wifiMac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid));
//
//   NetDeviceContainer apDevice;
//   apDevice = wifiHelper.Install (wifiPhy, wifiMac, apWifiNode);
//
//   /* Configure STA */
//   wifiMac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid));
//
//   NetDeviceContainer staDevices;
//   staDevices = wifiHelper.Install (wifiPhy, wifiMac, staWifiNode);
//
//   /* Mobility model */
//   MobilityHelper mobility;
//   Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
//   positionAlloc->Add (Vector (0.0, 0.0, 0.0));
//   positionAlloc->Add (Vector (1.0, 1.0, 0.0));
//
//   mobility.SetPositionAllocator (positionAlloc);
//   mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
//   mobility.Install (apWifiNode);
//   mobility.Install (staWifiNode);
//
//   // install internet stack
//   InternetStackHelper stackHelper;
//   // Ipv4ListRoutingHelper listRoutingHelper;
//   // Ipv4StaticRoutingHelper staticRoutingHelper;
//   // listRoutingHelper.Add(staticRoutingHelper, 10);
//   // stackHelper.SetRoutingHelper(listRoutingHelper);
//   stackHelper.Install (networkNodes);
//
//   Ipv4AddressHelper address;
//   address.SetBase ("10.10.0.0", "255.255.255.0");
//   Ipv4InterfaceContainer apInterface;
//   apInterface = address.Assign (apDevice);
//   Ipv4InterfaceContainer staInterface;
//   staInterface = address.Assign (staDevices);
//
//
//   EnableLogging ();
//
//   Ptr<Ipv4> ipv4s = apWifiNode->GetObject<Ipv4>();
//   Ipv4Address addrs = ipv4s->GetAddress(1,0).GetLocal();
//   Address serverAddress(InetSocketAddress(addrs, 4000));
//   Ptr<Ipv4> ipv4c = staWifiNode->GetObject<Ipv4>();
//   Ipv4Address addrc = ipv4c->GetAddress(1,0).GetLocal();
//   Address clientAddress(InetSocketAddress(addrc, 4000));
//   //std::cout << "Node Id:"<< staWifiNode->GetId() << "client sta Ip: " << addrc << "\nNode Id:"<< apWifiNode->GetId() << "server ap Ip: " << addrs << std::endl;
//
//   // // config routing
//   // Ptr<Ipv4StaticRouting> serverRouting = GetNodeStaticRoutingProtocol(apWifiNode);
//   // serverRouting->AddHostRouteTo(addrc, addrc, 1);
//   // Ptr<Ipv4StaticRouting> clientRouting = GetNodeStaticRoutingProtocol(staWifiNode);
//   // clientRouting->AddHostRouteTo(addrs, addrs, 1);
//
//       // ----------------- wired connectioin: -----------------
//       NodeContainer linkedNodes;
//       linkedNodes.Add(apWifiNode);
//       linkedNodes.Add(staWifiNode);
//
//       PointToPointHelper pointToPoint;
//       pointToPoint.SetDeviceAttribute ("DataRate", DataRateValue (DataRate("3Mbps")));
//       pointToPoint.SetChannelAttribute ("Delay", TimeValue (Time("5ms")));
//
//       TrafficControlHelper tchRed;
//       tchRed.SetRootQueueDisc ("ns3::RedQueueDisc",
//                                "MeanPktSize", UintegerValue(1424),
//                                "LinkBandwidth", DataRateValue (DataRate("3Mbps")),
//                                "LinkDelay", TimeValue (Time("5ms")));
//
//       pointToPoint.SetQueue("ns3::DropTailQueue", "MaxPackets", UintegerValue(1));
//
//       NetDeviceContainer linkedDevices;
//       linkedDevices = pointToPoint.Install (linkedNodes);
//
//       tchRed.Install(linkedDevices);
//       // address.SetBase ("10.10.1.0", "255.255.255.0");
//       address.Assign(linkedDevices);
//
//       Ptr<Ipv4> ipv4s_wired = apWifiNode->GetObject<Ipv4>();
//       Ipv4Address addrs_wired = ipv4s_wired->GetAddress(2, 0).GetLocal();
//       // Address serverAddress(InetSocketAddress(addrs_wired, 4001));
//       Ptr<Ipv4> ipv4c_wired = staWifiNode->GetObject<Ipv4>();
//       Ipv4Address addrc_wired = ipv4c_wired->GetAddress(2, 0).GetLocal();
//       // Address clientAddress(InetSocketAddress(addrc_wired, 4001));
//       // // config routing
//       // Ptr<Ipv4StaticRouting> serverRouting_wired = GetNodeStaticRoutingProtocol(apWifiNode);
//       // serverRouting_wired->AddHostRouteTo(addrc_wired, addrc_wired, 2);
//       // Ptr<Ipv4StaticRouting> clientRouting_wired = GetNodeStaticRoutingProtocol(staWifiNode);
//       // clientRouting_wired->AddHostRouteTo(addrs_wired, addrs_wired, 2);
//
//       // ----------------- wired connectioin: end -----------------
//
//   Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
//   Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("debug-routing-mptcp.routes", std::ios::out);
//   Ipv4GlobalRoutingHelper g;
//   g.PrintRoutingTableAllAt (Seconds (0.5), routingStream);
//
//   // Create application
//   Ptr<MpOnOffApplication> mpOnOff = CreateObject<MpOnOffApplication>();
//   mpOnOff->SetAttribute("Protocol", StringValue("ns3::MpTcpSocketFactory"));
//   mpOnOff->SetAttribute("Remote", AddressValue (clientAddress));
//   mpOnOff->SetAttribute("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
//   mpOnOff->SetAttribute("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));
//   mpOnOff->SetAttribute("DataRate", DataRateValue (DataRate("3Mbps")));
//   mpOnOff->SetAttribute("PacketSize", UintegerValue (1424));
//
//   // Install on server
//   apWifiNode->AddApplication(mpOnOff);
//
//   // Install on client (sta)
//   PacketSinkHelper sinkHelper ("ns3::MpTcpSocketFactory", clientAddress);
//   ApplicationContainer sinkApp = sinkHelper.Install (staWifiNode);
//
//   /* Start Applications */
//   // sink = sinkApp.Get(0);
//   sinkApp.Start (Seconds (0.0));
//
//   for(int i = 0;i < 11;i++){
//     Simulator::Schedule(Seconds(i/1.0), &PrintMonitorStates);
//   }
//   // mpOnOff.Start (Seconds (1.0));
//   // Simulator::Schedule (Seconds (1.1), &CalculateThroughput);
//
// ////////////////**************************///////////////////////
//
//   /* Enable Traces */
//   if (pcapTracing)
//     {
//       wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
//       wifiPhy.EnablePcap ("AccessPoint", apDevice);
//       wifiPhy.EnablePcap ("Station", staDevices);
//     }
//   /* Start Simulation */
//   Simulator::Stop (Seconds (simulationTime + 1));
//   Simulator::Run ();
//   Simulator::Destroy ();
//
//   // double averageThroughput = ((sink->GetTotalRx() * 8) / (1e6  * simulationTime));
//   // if (averageThroughput < 50)
//   //   {
//   //     NS_LOG_ERROR ("Obtained throughput is not in the expected boundaries!");
//   //     exit (1);
//   //   }
//   //std::cout << "\nAverage throughtput: " << averageThroughput << " Mbit/s" << std::endl;
//   return 0;
// }
