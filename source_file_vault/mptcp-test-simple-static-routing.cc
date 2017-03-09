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
#include "rl_ClientSocket.h"
#include "rl_SocketException.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <assert.h>

const int MAXHOSTNAME = 200;
const int MAXCONNECTIONS = 5;
const int MAXRECV = 500;

void SendIntegerToTensorflow(int number){
  try{
    rl::ClientSocket client_socket("localhost", 12345);
    assert(number > 0);
    int count = 0;
    while(count < number){
      std::cout << count << std::endl;
      std::string reply;
        try{
          client_socket << static_cast<std::stringstream*>(&(std::stringstream() << count))->str();
          count++;
          client_socket >> reply;
        }
        catch(rl::SocketException& ){
          std::cout << "We got an SocketException!" << std::endl;
        }
        std::cout << "We received this response from the server: \"" << reply << "\"\n";
    }
  }
  catch(rl::SocketException& e){
    std::cout << "Exception was caught:" << e.description() << "\n";
  }
}

using namespace ns3;

void TraceMacRx(Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet)
{
  PppHeader pppHeader;
  Ipv4Header ipheader;
  TcpHeader tcpHeader;

  Ptr<Packet> copy = packet->Copy();
  copy->RemoveHeader(pppHeader);

  if (pppHeader.GetProtocol() == 0x0021)
  {
    copy->RemoveHeader(ipheader);

    if (ipheader.GetProtocol() == TcpL4Protocol::PROT_NUMBER)
    {
      copy->RemoveHeader(tcpHeader);

      MpTcpSubflowTag subflowTag;
      bool found = copy->PeekPacketTag(subflowTag);

      int subflowId = -1;
      if (found){
        subflowId =  subflowTag.GetSubflowId();
      }

      //Figure out if this is a FIN packet, or SYN packet
      bool isFin = tcpHeader.GetFlags() & TcpHeader::FIN;
      bool isSyn = tcpHeader.GetFlags() & TcpHeader::SYN;

      (*stream->GetStream()) << Simulator::Now().GetNanoSeconds() << " 0 1 "
      << subflowId << " "
      << tcpHeader.GetSequenceNumber() << " " << tcpHeader.GetAckNumber()
      << " " << copy->GetSize() << " " << packet->GetSize()
      << " " << isSyn << " " << isFin << endl;

    }
  }
}

void TraceMacTx(Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet)
{
  PppHeader pppHeader;
  Ipv4Header ipheader;
  TcpHeader tcpHeader;

  Ptr<Packet> copy = packet->Copy();
  copy->RemoveHeader(pppHeader);

  if (pppHeader.GetProtocol() == 0x0021)
  {
    copy->RemoveHeader(ipheader);

    if (ipheader.GetProtocol() == TcpL4Protocol::PROT_NUMBER)
    {
      copy->RemoveHeader(tcpHeader);

      MpTcpSubflowTag subflowTag;
      bool found = copy->PeekPacketTag(subflowTag);

      int subflowId = -1;
      if (found){
        subflowId =  subflowTag.GetSubflowId();
      }


      //Figure out if this is a FIN packet, or SYN packet
      bool isFin = tcpHeader.GetFlags() & TcpHeader::FIN;
      bool isSyn = tcpHeader.GetFlags() & TcpHeader::SYN;

      (*stream->GetStream()) << Simulator::Now().GetNanoSeconds() << " 1 1 "
      << subflowId << " "
      << tcpHeader.GetSequenceNumber() << " " << tcpHeader.GetAckNumber()
      << " " << copy->GetSize() << " " << packet->GetSize()
      << " " << isSyn << " " << isFin << endl;

    }
  }
}

void TraceQueueItemDrop(Ptr<OutputStreamWrapper> stream, Ptr<const QueueItem> item)
{
  Ptr<const Ipv4QueueDiscItem> qitem = StaticCast<const Ipv4QueueDiscItem>(item);

  Ipv4Header ipheader = qitem->GetHeader();
  TcpHeader tcpHeader;

  if ((qitem->GetProtocol() == Ipv4L3Protocol::PROT_NUMBER)
      && (ipheader.GetProtocol() == TcpL4Protocol::PROT_NUMBER))
  {
    Ptr<Packet> copy = item->GetPacket();
    copy->RemoveHeader(tcpHeader);

    //Figure out if this is a FIN packet, or SYN packet
    bool isFin = tcpHeader.GetFlags() & TcpHeader::FIN;
    bool isSyn = tcpHeader.GetFlags() & TcpHeader::SYN;

    (*stream->GetStream()) << Simulator::Now().GetNanoSeconds()  << " "
    << tcpHeader.GetSequenceNumber() << " " << tcpHeader.GetAckNumber()
    << " " << isSyn << " " << isFin << endl;

  }
}

void CheckAndCreateDirectory(string path)
{
  if(access(path.c_str(), F_OK ) == -1 ){
    const int error = mkdir(path.c_str(), S_IRWXU | S_IRWXG |  S_IROTH);

    if(error == -1){
      NS_FATAL_ERROR("Could not create directory " << path);
    }
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

void EnableLogging ()
{
  /*LogComponentEnable("TcpL4Protocol", LOG_LEVEL_ALL);
  LogComponentEnable("TcpSocketBase", LOG_LEVEL_ALL);
  LogComponentEnable("MpTcpMetaSocket", LOG_LEVEL_ALL);
  LogComponentEnable("MpTcpSubflow", LOG_LEVEL_ALL);

  LogComponentEnable ("Ipv4L3Protocol", LOG_LEVEL_ALL);
  LogComponentEnable("PointToPointNetDevice", LOG_LEVEL_ALL);
  LogComponentEnable ("Ipv4EndPoint", LOG_LEVEL_ALL);*/
}

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

void ConfigureTracing (const string& outputDir, const NodeContainer& clients,
                       const NodeContainer& switches, const NodeContainer& servers)
{
  //Create an output directory
  CheckAndCreateDirectory(outputDir);

  stringstream devicePath;
  devicePath << "/NodeList/" << clients.Get(0)->GetId() << "/DeviceList/*/$ns3::PointToPointNetDevice/";

  stringstream tfile;
  tfile << outputDir << "/mptcp_client";
  Ptr<OutputStreamWrapper> throughputFile = Create<OutputStreamWrapper>(tfile.str(), std::ios::out);
  //Write the column labels into the file
  *(throughputFile->GetStream()) << "timestamp send connection subflow seqno ackno size psize isSyn isFin" << endl;

  Config::ConnectWithoutContext(devicePath.str() + "MacRx", MakeBoundCallback(TraceMacRx, throughputFile));
  Config::ConnectWithoutContext(devicePath.str() + "MacTx", MakeBoundCallback(TraceMacTx, throughputFile));

  uint32_t serverId = servers.Get(0)->GetId();
  devicePath.str("");
  devicePath << "/NodeList/" << serverId << "/DeviceList/*/$ns3::PointToPointNetDevice/";

  stringstream sfile;
  sfile << outputDir << "/mptcp_server";
  Ptr<OutputStreamWrapper> serverFile = Create<OutputStreamWrapper>(sfile.str(), std::ios::out);
  //Write the column labels into the file
  *(serverFile->GetStream()) << "timestamp send connection subflow seqno ackno size psize isSyn isFin" << endl;
  Config::ConnectWithoutContext(devicePath.str() + "MacTx", MakeBoundCallback(TraceMacTx, serverFile));
  Config::ConnectWithoutContext(devicePath.str() + "MacRx", MakeBoundCallback(TraceMacRx, serverFile));

  stringstream dfile;
  dfile << outputDir << "/mptcp_drops";
  Ptr<OutputStreamWrapper> dropsFile = Create<OutputStreamWrapper>(dfile.str(), std::ios::out);
  //Write the column labels into the file
  *(dropsFile->GetStream()) << "timestamp seqno ackno isSyn isFin" << endl;

  Config::ConnectWithoutContext("/NodeList/*/$ns3::TrafficControlLayer/RootQueueDiscList/*/Drop",
                                MakeBoundCallback(TraceQueueItemDrop, dropsFile));


  uint32_t clientId = clients.Get(0)->GetId();
  uint32_t switchId = switches.Get(0)->GetId();

  cout << "client node is " << clientId << " switch id " << switchId << " server id " << serverId << endl;

  //Print the nodes' routing tables
  //Print the nodes' routing tables
  for (uint32_t  i = 0; i < switches.GetN(); ++i)
  {
    PrintRoutingTable(switches.Get(i), outputDir, "switch" + to_string(i));
  }
  for(uint32_t i = 0; i < clients.GetN(); ++i)
  {
    PrintRoutingTable(clients.Get(i), outputDir, "cl" + to_string(i));
  }
  for(uint32_t i = 0; i < servers.GetN(); ++i)
  {
    PrintRoutingTable(servers.Get(i), outputDir, "srv" + to_string(i));
  }
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

  /*
   * Version 0.1
   *
   *          -switch 0   ---  switch 2 -
   *         /            \   /           \
   * server                \              client
   *        \            /   \           /
   *         - seitch 1   ---  switch 3 -
   *
   *
   */

  //Create the nodes in the topology, and install the internet stack on them
  clients.Create(1);
  stackHelper.Install(clients);

  switches.Create(4);
  stackHelper.Install(switches);

  //Create the servers and install the internet stack on them
  servers.Create(1);
  stackHelper.Install(servers);

  //Create the address helper
  Ipv4AddressHelper addressHelper;
  addressHelper.SetBase("10.10.0.0", "255.255.255.0");

  Ipv4InterfaceContainer serverInterfaces;
  Ipv4InterfaceContainer switchesInterfaces;
  Ipv4InterfaceContainer clientInterfaces;

  addressHelper.Assign(PointToPointCreate(servers.Get(0), switches.Get(0), DataRate(linkRate.GetBitRate() * 2), delay, packetSize));
  addressHelper.Assign(PointToPointCreate(servers.Get(0), switches.Get(1), DataRate(linkRate.GetBitRate() * 2), delay, packetSize));
  addressHelper.Assign(PointToPointCreate(switches.Get(0), switches.Get(3), DataRate(linkRate.GetBitRate() * 2), delay, packetSize));
  addressHelper.Assign(PointToPointCreate(switches.Get(0), switches.Get(2), DataRate(linkRate.GetBitRate() * 2), delay, packetSize));
  addressHelper.Assign(PointToPointCreate(switches.Get(1), switches.Get(3), DataRate(linkRate.GetBitRate() * 2), delay, packetSize));
  addressHelper.Assign(PointToPointCreate(switches.Get(1), switches.Get(2), DataRate(linkRate.GetBitRate() * 2), delay, packetSize));
  addressHelper.Assign(PointToPointCreate(switches.Get(3), clients.Get(0), DataRate(linkRate.GetBitRate() * 2), delay, packetSize));
  addressHelper.Assign(PointToPointCreate(switches.Get(2), clients.Get(0), DataRate(linkRate.GetBitRate() * 2), delay, packetSize));

  Ptr<Ipv4> ipv4 = clients.Get(0)->GetObject<Ipv4>();
  Ipv4InterfaceAddress iaddr = ipv4->GetAddress(1,0);
  remoteClient = iaddr.GetLocal();
  std::cout<<"Ip address of client: "<<remoteClient<<endl;
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

  enum TopologyType
  {
    bottleneck = 0,
    nobottleneck,
    otherflow
  };

  enum AppType
  {
    onoff = 0,
    filetransfer = 1
  };

  uint32_t interfaceCount = 2;
  uint32_t type = 0;
  uint32_t appType = 1;
  string outputDir = "mptcp_test";

  CommandLine cmd;
  cmd.AddValue("interfaces", "The number of interfaces", interfaceCount);
  cmd.AddValue("topologyType", "The type of topology to use", type);
  cmd.AddValue("outputDir", "The output directory to write the logs to.", outputDir);
  cmd.AddValue("appType", "The type of application to run", appType);
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

  //Enable logging
  EnableLogging ();

  SetConfigDefaults(linkRate, linkDelay, interfaceCount, segmentSize, segmentSizeWithoutHeaders, queueSize);

  //Create the nodes in the topology, and install the internet stack on them
  NodeContainer clients;
  NodeContainer switches;
  NodeContainer servers;

  Ipv4Address remoteClient;
  std::cout << "Hi, 1" << endl;
  CreateMultipleFlowsSingleBottleneck(interfaceCount, segmentSizeWithoutHeaders,
                                      rate, delay,
                                      servers, switches, clients,
                                      remoteClient);
  //Create and install the applications on the server and client
  InstallFileTransferApplications(servers, clients, remoteClient,
                                    segmentSizeWithoutHeaders,
                                    queueSize);

  //Populate the IP routing tables
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  //Create an output directory and configure tracing
  ConfigureTracing(outputDir, clients, switches, servers);

  //Set the simulator stop time
  Simulator::Stop (Seconds(20.0));

  Simulator::Schedule(Seconds(3.0), &SendIntegerToTensorflow, 3);
  //Begin the simulation

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
