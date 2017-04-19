#include "mp-onoff-application.h"

#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include "file-transfer-helper.h"
#include "file-transfer-application.h"

namespace ns3{

Ptr<Application> CreateApplication (Address& remoteAddress, DataRate dataRate, uint32_t packetSize)
{
  Ptr<MpOnOffApplication> onOff = CreateObject<MpOnOffApplication>();
  onOff->SetAttribute("Protocol", StringValue("ns3::MpTcpSocketFactory"));
  onOff->SetAttribute("Remote", AddressValue (remoteAddress));
  onOff->SetAttribute("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
  onOff->SetAttribute("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
  onOff->SetAttribute("DataRate", DataRateValue (dataRate));
  onOff->SetAttribute("PacketSize", UintegerValue (packetSize));
  return onOff;
}

void InstallOnOffApplications(NodeContainer& servers, NodeContainer& clients, uint32_t packetSize)
{
  //Create and install the applications on the server and client
  NS_ASSERT(servers.GetN() == clients.GetN());

  int portNum = 4000;
  for(int i = 0; i < clients.GetN();i++){
    Ptr<Ipv4> ipv4 = clients.Get(i)->GetObject<Ipv4>(); // Interface number of Ipv4 interface = 1 (0 is 0.0.0.0?); addressIndex = 0
    Ipv4InterfaceAddress iaddr = ipv4->GetAddress(1,0);
    Ipv4Address addr = iaddr.GetLocal();
    Address remoteAddress(InetSocketAddress(addr, portNum));

    // Create application
    Ptr<Application> mpOnOff = CreateApplication(remoteAddress, DataRate("0.3Mbps"), packetSize);

    // Install on server
    servers.Get(i)->AddApplication(mpOnOff);

    // Install on client
    PacketSinkHelper packetSink("ns3::MpTcpSocketFactory", remoteAddress);
    packetSink.Install(clients.Get(i));
    portNum++;
  }
  // Address remoteAddress(InetSocketAddress(peer, portNum));
  //
  // Ptr<Application> mpOnOff = CreateApplication(remoteAddress, DataRate("0.3Mbps"), packetSize);
  // servers.Get(0)->AddApplication(mpOnOff);
  //
  // //PacketSinkHelper packetSink("ns3::MpTcpSocketFactory", remoteAddress);
  // Address portAddress(InetSocketAddress(Ipv4Address::GetAny(), portNum));
  // PacketSinkHelper packetSink("ns3::MpTcpSocketFactory", portAddress);
  // packetSink.Install(clients);

  // if (servers.GetN() == 2)
  // {
  //   int portNum = 4020;
  //   Address tcpRemote(InetSocketAddress(peer, portNum));
  //
  //   OnOffHelper onOff("ns3::TcpSocketFactory", tcpRemote);
  //   onOff.SetConstantRate(DataRate("2Mbps"), packetSize);
  //   onOff.Install(servers.Get(1));
  //
  //   PacketSinkHelper packetSink("ns3::TcpSocketFactory", tcpRemote);
  //   packetSink.Install(clients);
  // }
}

void InstallFileTransferApplications(NodeContainer& servers, NodeContainer& clients, uint32_t packetSize, uint32_t queueSize)
{
  //Create and install the applications on the server and client
  NS_ASSERT(servers.GetN() == clients.GetN());

  int portNum = 4000;

  for(int i = 0; i < clients.GetN();i++){
    // Create application
    // Interface number of Ipv4 interface = 1 (0 is 0.0.0.0?); addressIndex = 0
    Ptr<Ipv4> ipv4 = clients.Get(i)->GetObject<Ipv4>(); // Interface number of Ipv4 interface = 1 (0 is 0.0.0.0?); addressIndex = 0
    Ipv4InterfaceAddress iaddr = ipv4->GetAddress(1,0);
    Ipv4Address addr = iaddr.GetLocal();
    Address remoteAddress(InetSocketAddress(addr, portNum));
    FileTransferHelper fileHelper(remoteAddress);
    fileHelper.SetAttribute("Protocol", TypeIdValue(MpTcpSocketFactory::GetTypeId()));
    fileHelper.SetAttribute("FileSize", UintegerValue(10e6));

    // Install on server
    ApplicationContainer apps = fileHelper.Install(servers.Get(i));

    // Install on client
    PacketSinkHelper packetSink("ns3::TcpSocketFactory", remoteAddress);
    packetSink.Install(clients.Get(i));
    // //Set the tx buffer size to the interface queue size
    // Ptr<Node> tcpServer = servers.Get(1);
    // Ptr<TcpSocket> socket = DynamicCast<TcpSocket>(StaticCast<FileTransferApplication>(apps.Get(0))->GetSocket());
    // //socket->SetAttribute("SndBufSize", UintegerValue(queueSize));
    portNum++;
  }

  // //PacketSinkHelper packetSink("ns3::MpTcpSocketFactory", remoteAddress);
  // Address portAddress(InetSocketAddress(Ipv4Address::GetAny(), portNum));
  // PacketSinkHelper packetSink("ns3::MpTcpSocketFactory", portAddress);
  // packetSink.Install(clients);
  //
  // if (servers.GetN() == 2)
  // {
  //   int portNum = 4020;
  //   Address tcpRemote(InetSocketAddress(peer, portNum));
  //
  //   fileHelper.SetAttribute("Remote", AddressValue(tcpRemote));
  //   fileHelper.SetAttribute("Protocol", TypeIdValue(TcpSocketFactory::GetTypeId()));
  //   ApplicationContainer apps = fileHelper.Install(servers.Get(1));
  //
  //   PacketSinkHelper packetSink("ns3::TcpSocketFactory", tcpRemote);
  //   packetSink.Install(clients);
  // }

}

};
