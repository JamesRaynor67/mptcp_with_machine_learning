//
//  mptcp-test.cc
//  ns3
//
//  Created by Lynne Salameh on 17/6/16.
//

#include <stdio.h>
#include <string>
#include <limits>
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include <sys/stat.h>
#include <unistd.h>
#include "mp-onoff-application.h"
#include "file-transfer-helper.h"
#include "file-transfer-application.h"
#include "ns3/ipv4-global-routing-helper.h"
#include <vector>

using namespace std;
using namespace ns3;

void CheckAndCreateDirectory(string path)
{
  if(access(path.c_str(), F_OK ) == -1 ){
    const int error = mkdir(path.c_str(), S_IRWXU | S_IRWXG |  S_IROTH);

    if(error == -1){
      NS_FATAL_ERROR("Could not create directory " << path);
    }
  }
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

Ptr<Application> CreateApplication (Address& remoteAddress, DataRate dataRate, uint32_t packetSize)
{
  Ptr<MpOnOffApplication> onOff = CreateObject<MpOnOffApplication>();
  onOff->SetAttribute("Protocol", StringValue("ns3::MpTcpSocketFactory"));
  onOff->SetAttribute("Remote", AddressValue (remoteAddress));
  onOff->SetAttribute("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1000]"));
  onOff->SetAttribute("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onOff->SetAttribute("DataRate", DataRateValue (dataRate));
  onOff->SetAttribute("PacketSize", UintegerValue (packetSize));
  return onOff;
}

void SetConfigDefaults (string linkRate, string linkDelay, uint32_t interfaces,
                        uint32_t segmentSize, uint32_t segmentSizeWithoutHeaders,
                        uint32_t queueSize)
{
  //The bandwidth delay product
  //uint32_t bdp = DataRate(linkRate).GetBitRate() * Time(linkDelay).GetSeconds() * 4;
  //uint32_t bdpBytes = bdp/8;

  //TCP configuration
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (segmentSizeWithoutHeaders));

  //Slow start threshold
  Config::SetDefault("ns3::TcpSocket::InitialSlowStartThreshold", UintegerValue(numeric_limits<uint32_t>::max()));

  //Set the receive window size to the maximum possible
  Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue(1<<30));

  //Disable the timestamp option
  Config::SetDefault("ns3::TcpSocketImpl::Timestamp", BooleanValue(false));

  //Set the mptcp option
  Config::SetDefault("ns3::TcpSocketImpl::EnableMpTcp", BooleanValue(true));

  //Set the initial congestion window to be larger than duplicate ack threshold
  Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue(4));

  //Config::SetDefault("ns3::Queue::Mode", EnumValue(Queue::QUEUE_MODE_BYTES));
  //Config::SetDefault("ns3::Queue::MaxBytes", UintegerValue(queueSize));

  Config::SetDefault("ns3::RedQueueDisc::Mode", EnumValue(Queue::QUEUE_MODE_BYTES));
  Config::SetDefault("ns3::RedQueueDisc::MinTh", DoubleValue(queueSize * 0.33));
  Config::SetDefault("ns3::RedQueueDisc::MaxTh", DoubleValue(queueSize * 0.66));
  Config::SetDefault("ns3::RedQueueDisc::QueueLimit", UintegerValue(queueSize));
  Config::SetDefault("ns3::RedQueueDisc::QW", DoubleValue(1));
  Config::SetDefault("ns3::RedQueueDisc::LInterm", DoubleValue(10));
  Config::SetDefault("ns3::RedQueueDisc::Wait", BooleanValue(false));

  //Set the send buffer to be the n * queue size to accomodate n subflows
  //Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (interfaces * queueSize));
  Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (numeric_limits<uint32_t>::max()));

  Config::SetDefault ("ns3::TcpSocket::ConnTimeout", TimeValue(Seconds(2.0)));
  Config::SetDefault("ns3::ArpCache::AliveTimeout", TimeValue(Seconds(120 + 1)));

  Config::SetDefault("ns3::MpTcpMetaSocket::TagSubflows", BooleanValue(true));

  //Config::SetDefault("ns3::Ipv4::WeakEsModel", BooleanValue(false));
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
  // InternetStackHelper stackHelper = GetInternetStackHelper();
  InternetStackHelper stackHelper;

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
}

void InstallOnOffApplications(NodeContainer& servers, NodeContainer& clients,
                              const Ipv4Address& peer, uint32_t packetSize)
{
  //Create and install the applications on the server and client
  int portNum = 4000;
  Address remoteAddress(InetSocketAddress(peer, portNum));

  Ptr<Application> mpOnOff = CreateApplication(remoteAddress, DataRate("10Mbps"), packetSize);
  servers.Get(0)->AddApplication(mpOnOff);

  //PacketSinkHelper packetSink("ns3::MpTcpSocketFactory", remoteAddress);
  Address portAddress(InetSocketAddress(Ipv4Address::GetAny(), portNum));
  PacketSinkHelper packetSink("ns3::MpTcpSocketFactory", portAddress);
  packetSink.Install(clients);

  if (servers.GetN() == 2)
  {
    int portNum = 4020;
    Address tcpRemote(InetSocketAddress(peer, portNum));

    OnOffHelper onOff("ns3::TcpSocketFactory", tcpRemote);
    onOff.SetConstantRate(DataRate("20Mbps"), packetSize);
    onOff.Install(servers.Get(1));

    PacketSinkHelper packetSink("ns3::TcpSocketFactory", tcpRemote);
    packetSink.Install(clients);
  }
}

void InstallFileTransferApplications(NodeContainer& servers, NodeContainer& clients,
                                     const Ipv4Address& peer, uint32_t packetSize, uint32_t queueSize)
{
  //Create and install the applications on the server and client
  int portNum = 4000;
  Address remoteAddress(InetSocketAddress(peer, portNum));

  FileTransferHelper fileHelper(remoteAddress);
  fileHelper.SetAttribute("Protocol", TypeIdValue(MpTcpSocketFactory::GetTypeId()));
  fileHelper.SetAttribute("FileSize", UintegerValue(10e6));

  fileHelper.Install(servers.Get(0));

  //PacketSinkHelper packetSink("ns3::MpTcpSocketFactory", remoteAddress);
  Address portAddress(InetSocketAddress(Ipv4Address::GetAny(), portNum));
  PacketSinkHelper packetSink("ns3::MpTcpSocketFactory", portAddress);
  packetSink.Install(clients);

  if (servers.GetN() == 2)
  {
    int portNum = 4020;
    Address tcpRemote(InetSocketAddress(peer, portNum));

    fileHelper.SetAttribute("Remote", AddressValue(tcpRemote));
    fileHelper.SetAttribute("Protocol", TypeIdValue(TcpSocketFactory::GetTypeId()));
    ApplicationContainer apps = fileHelper.Install(servers.Get(1));

    PacketSinkHelper packetSink("ns3::TcpSocketFactory", tcpRemote);
    packetSink.Install(clients);

    //Set the tx buffer size to the interface queue size
    Ptr<Node> tcpServer = servers.Get(1);
    Ptr<TcpSocket> socket = DynamicCast<TcpSocket>(StaticCast<FileTransferApplication>(apps.Get(0))->GetSocket());
    //socket->SetAttribute("SndBufSize", UintegerValue(queueSize));
  }
}

int main(int argc, char* argv[])
{
  uint32_t interfaceCount = 2;
  string outputDir = "mptcp_output";

  CommandLine cmd;
  cmd.AddValue("interfaces", "The number of interfaces", interfaceCount);
  cmd.AddValue("outputDir", "The output directory to write the logs to.", outputDir);
  cmd.Parse(argc, argv);

  string linkRate = "10Mbps";
  string linkDelay = "10ms";

  //For now let's assume the maximum size of mptcp option is 34 bytes
  uint32_t tcpOptionSize = 34;
  uint32_t headersSize = 20 + 20 + 2 + tcpOptionSize; //ipheader + tcpheader + pppheader + tcp options
  uint32_t segmentSize = 1500;
  uint32_t segmentSizeWithoutHeaders = segmentSize - headersSize;

  DataRate rate(linkRate);
  Time delay(linkDelay);

  uint32_t bdp = rate.GetBitRate() * delay.GetSeconds() * 4;
  uint32_t bdpBytes = bdp/8;
  uint32_t queueSize = 1.5 * max<uint32_t>(bdpBytes, 10 * segmentSize);

  SetConfigDefaults(linkRate, linkDelay, interfaceCount, segmentSize, segmentSizeWithoutHeaders, queueSize);

  //Create the nodes in the topology, and install the internet stack on them
  NodeContainer clients;
  NodeContainer switches;
  NodeContainer servers;

  Ipv4Address remoteClient;

  /*
   * Server has two possible paths to client.
   * Note that two switches are necessary to make
   * this work without the ADD_ADDR capabilities implemented
   * Also the packet sink bound endpoint has Ipv4Address::GetAny()
   *
   ------           ------           ------
  |      | ------  |      | ------  |      |
  |      |         |      |         |      |
  |      |          ------          |      |
  |      |          Switch1         |      |
  |      |          ------          |      |
  |      | ------  |      | ------  |      |
  |      |         |      |         |      |
   ------           ------           ------
   Server           Switch2          Client
   *
   */
  CreateMultipleFlowsNoBottleneck(interfaceCount, segmentSizeWithoutHeaders,
                                  rate, delay,
                                  servers, switches, clients,
                                  remoteClient);

  //Create and install the applications on the server and client
  InstallFileTransferApplications(servers, clients, remoteClient, segmentSizeWithoutHeaders, queueSize);

  //Populate the IP routing tables
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Ipv4GlobalRoutingHelper g;
  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("dynamic-global-routing.routes", std::ios::out);
  g.PrintRoutingTableAllAt (Seconds (6), routingStream);
  //Set the simulator stop time
  Simulator::Stop (Seconds(10.0));

  //Begin the simulation
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
