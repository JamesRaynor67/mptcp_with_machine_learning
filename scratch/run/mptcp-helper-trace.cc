#include <sstream>

#include "mptcp-helper-system.h"
#include "mptcp-helper-router.h"
#include "mptcp-helper-trace.h"
#include "ns3/rl-data-interface.h"

#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/core-module.h"

namespace ns3{

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

      // this is the only difference with TraceMacTx   send connection
      (*stream->GetStream()) << Simulator::Now().GetNanoSeconds() << ",0,1,"
      << subflowId << ","
      << tcpHeader.GetSequenceNumber() << "," << tcpHeader.GetAckNumber()
      << "," << copy->GetSize() << "," << packet->GetSize()
      << "," << isSyn << "," << isFin << endl;

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

      // this is the only difference with TraceMacRx   send connection
      (*stream->GetStream()) << Simulator::Now().GetNanoSeconds() << ",1,1,"
      << subflowId << ","
      << tcpHeader.GetSequenceNumber() << "," << tcpHeader.GetAckNumber()
      << "," << copy->GetSize() << "," << packet->GetSize()
      << "," << isSyn << "," << isFin << endl;

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

    (*stream->GetStream()) << Simulator::Now().GetNanoSeconds()  << ","
    << tcpHeader.GetSequenceNumber() << "," << tcpHeader.GetAckNumber()
    << "," << isSyn << "," << isFin << endl;

  }
}

void TraceMonitorStates(const string& outputDir){
  //Create flow monitor
  static FlowMonitorHelper flowmon;
  static Ptr<FlowMonitor> monitor = flowmon.InstallAll();;
  static bool initialized = false;
  static Ptr<OutputStreamWrapper> logFile;

  if(!initialized){
    // monitor = flowmon.InstallAll();
    logFile = Create<OutputStreamWrapper>(outputDir + "/mptcp_server_cWnd", std::ios::out);
    *(logFile->GetStream()) << "Timestamp,FlowId,From,To,TxPackets,TxBytes,RxPackets,RxBytes,DelaySum,JitterSum,LostPacketSum,TTL_expire,Bad_checksum" << endl;
    initialized = true;
  }

  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
  //std::cout << "Hong Jiaming: 0 " << endl;
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i){
    //std::cout << "Hong Jiaming: 1 " << i->first << endl;
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
    //std::cout << "Hong Jiaming: 2 " << i->first << endl;
    *(logFile->GetStream()) << Simulator::Now().GetNanoSeconds() << ","
                            << i->first << ","
                            << t.sourceAddress << ","
                            << t.destinationAddress << ","
                            << i->second.txPackets << ","
                            << i->second.txBytes << ","
                            << i->second.rxPackets << ","
                            << i->second.rxBytes << ","
                            << i->second.delaySum << ","
                            << i->second.jitterSum << ","
                            << i->second.lostPackets;
    if(i->second.packetsDropped.size() > Ipv4L3Protocol::DropReason::DROP_TTL_EXPIRED){
      *(logFile->GetStream()) << "," <<i->second.packetsDropped[Ipv4L3Protocol::DropReason::DROP_TTL_EXPIRED];
    }
    if(i->second.packetsDropped.size() > Ipv4L3Protocol::DropReason::DROP_BAD_CHECKSUM){
      *(logFile->GetStream()) << "," <<i->second.packetsDropped[Ipv4L3Protocol::DropReason::DROP_BAD_CHECKSUM];
    }
    *(logFile->GetStream()) << "\n";
    //std::cout << "Hong Jiaming: 3 " << i->first << endl;
  }
}

void PrintMonitorStates(void){
  //Create flow monitor
  static FlowMonitorHelper flowmon;
  static Ptr<FlowMonitor> monitor = flowmon.InstallAll();;

  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i){
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
    std::cout << "Timestamp: " << Simulator::Now().GetNanoSeconds() << ","
                    "FlowId: " << i->first << ","
                      "From: " << t.sourceAddress << ","
                        "To: " << t.destinationAddress << ","
                 "TxPackets: " << i->second.txPackets << ","
                   "TxBytes: " << i->second.txBytes << ","
                 "RxPackets: " << i->second.rxPackets << ","
                   "RxBytes: " << i->second.rxBytes << ","
                  "DelaySum: " << i->second.delaySum << ","
                 "JitterSum: " << i->second.jitterSum << ","
             "LostPacketSum: " << i->second.lostPackets;
    if(i->second.packetsDropped.size() > Ipv4L3Protocol::DropReason::DROP_TTL_EXPIRED){
      std::cout << ", TTL_expire: " <<i->second.packetsDropped[Ipv4L3Protocol::DropReason::DROP_TTL_EXPIRED];
    }
    if(i->second.packetsDropped.size() > Ipv4L3Protocol::DropReason::DROP_BAD_CHECKSUM){
      std::cout << ", Bad_checksum: " <<i->second.packetsDropped[Ipv4L3Protocol::DropReason::DROP_BAD_CHECKSUM];
    }
    std::cout << endl;
    //std::cout << "Hong Jiaming: 3 " << i->first << endl;
  }
  std::cout << endl;
}



void ConfigureTracing (const string& outputDir, const NodeContainer& server,
                       const NodeContainer& client, const NodeContainer& isps,
                       const NodeContainer& ixs)
{
  //Create an output directory
  CheckAndCreateDirectory(outputDir);

  // configure for clients
  stringstream devicePath;
  devicePath << "/NodeList/" << client.Get(0)->GetId() << "/DeviceList/*/$ns3::PointToPointNetDevice/";

  stringstream tfile;
  tfile << outputDir << "/mptcp_client";
  Ptr<OutputStreamWrapper> throughputFile = Create<OutputStreamWrapper>(tfile.str(), std::ios::out);
  //Write the column labels into the file
  *(throughputFile->GetStream()) << "timestamp,send,connection,subflow,seqno,ackno,size,psize,isSyn,isFin" << endl;

  Config::ConnectWithoutContext(devicePath.str() + "MacRx", MakeBoundCallback(TraceMacRx, throughputFile));
  Config::ConnectWithoutContext(devicePath.str() + "MacTx", MakeBoundCallback(TraceMacTx, throughputFile));

  // configure for server
  uint32_t serverId = server.Get(0)->GetId();
  devicePath.str("");
  devicePath << "/NodeList/" << serverId << "/DeviceList/*/$ns3::PointToPointNetDevice/";

  stringstream sfile;
  sfile << outputDir << "/mptcp_server";
  Ptr<OutputStreamWrapper> serverFile = Create<OutputStreamWrapper>(sfile.str(), std::ios::out);
  //Write the column labels into the file
  *(serverFile->GetStream()) << "timestamp,send,connection,subflow,seqno,ackno,size,psize,isSyn,isFin" << endl;
  Config::ConnectWithoutContext(devicePath.str() + "MacTx", MakeBoundCallback(TraceMacTx, serverFile));
  Config::ConnectWithoutContext(devicePath.str() + "MacRx", MakeBoundCallback(TraceMacRx, serverFile));

  // configure for droped packets
  stringstream dfile;
  dfile << outputDir << "/mptcp_drops";
  Ptr<OutputStreamWrapper> dropsFile = Create<OutputStreamWrapper>(dfile.str(), std::ios::out);
  //Write the column labels into the file
  *(dropsFile->GetStream()) << "timestamp,seqno,ackno,isSyn,isFin" << endl;

  Config::ConnectWithoutContext("/NodeList/*/$ns3::TrafficControlLayer/RootQueueDiscList/*/Drop",
                                MakeBoundCallback(TraceQueueItemDrop, dropsFile));


  uint32_t clientId = client.Get(0)->GetId();
  cout << "client node is: " << clientId << endl;
  cout << "server node is: " << serverId << endl;
  cout << "isps node are: ";
  for(int i = 0; i < isps.GetN(); ++i){
    cout << " " << isps.Get(i)->GetId();
  }
  cout << endl;
  cout << "ixs node are: ";
  for(int i = 0; i < ixs.GetN(); ++i){
    cout << " " << ixs.Get(i)->GetId();
  }
  cout << endl;

  //Print the nodes' routing tables
  //Print the nodes' routing tables
  for(uint32_t i = 0; i < server.GetN(); ++i)
  {
    PrintRoutingTable(server.Get(i), outputDir, "srv" + to_string(i));
  }
  for(uint32_t i = 0; i < client.GetN(); ++i)
  {
    PrintRoutingTable(client.Get(i), outputDir, "cl" + to_string(i));
  }
  for (uint32_t  i = 0; i < isps.GetN(); ++i)
  {
    PrintRoutingTable(isps.Get(i), outputDir, "isps" + to_string(i));
  }
  for (uint32_t  i = 0; i < ixs.GetN(); ++i)
  {
    PrintRoutingTable(ixs.Get(i), outputDir, "ixs" + to_string(i));
  }
}

};
