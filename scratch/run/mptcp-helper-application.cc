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
  onOff->SetAttribute("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=5.0]"));
  onOff->SetAttribute("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=3.0]"));
  onOff->SetAttribute("DataRate", DataRateValue (dataRate));
  onOff->SetAttribute("PacketSize", UintegerValue (packetSize));
  return onOff;
}

void InstallOnOffApplications(NodeContainer& servers, NodeContainer& clients,
                              const Ipv4Address& peer, uint32_t packetSize)
{
  //Create and install the applications on the server and client
  int portNum = 4000;
  Address remoteAddress(InetSocketAddress(peer, portNum));

  Ptr<Application> mpOnOff = CreateApplication(remoteAddress, DataRate("0.02Mbps"), packetSize);
  servers.Get(0)->AddApplication(mpOnOff);

  //PacketSinkHelper packetSink("ns3::MpTcpSocketFactory", remoteAddress);
  Address portAddress(InetSocketAddress(Ipv4Address::GetAny(), portNum));
  PacketSinkHelper packetSink("ns3::MpTcpSocketFactory", portAddress);
  packetSink.Install(clients);

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

};
