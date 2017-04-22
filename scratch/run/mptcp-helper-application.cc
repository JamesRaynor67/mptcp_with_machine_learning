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
  std::cout << "WTF InstallOnOffApplications 0" << endl;

  static int portNum = 4000;
  for(int i = 0; i < clients.GetN();i++){
    std::cout << "InstallOnOffApplications 1" << endl;
    Ptr<Ipv4> ipv4 = clients.Get(i)->GetObject<Ipv4>(); // Interface number of Ipv4 interface = 1 (0 is 0.0.0.0?); addressIndex = 0
    Ipv4InterfaceAddress iaddr = ipv4->GetAddress(1,0);
    Ipv4Address addr = iaddr.GetLocal();
    std::cout << "InstallOnOffApplications 2" << endl;
    Address remoteAddress(InetSocketAddress(addr, portNum));
    std::cout << "From: " << servers.Get(i)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal() << " To: " << addr << endl;

    // Create application
    Ptr<Application> mpOnOff = CreateApplication(remoteAddress, DataRate("0.3Mbps"), packetSize);

    // Install on server
    servers.Get(i)->AddApplication(mpOnOff);

    // Install on client
    PacketSinkHelper packetSink("ns3::MpTcpSocketFactory", remoteAddress);
    packetSink.Install(clients.Get(i));
    portNum++;
  }
}

void InstallFileTransferApplications(NodeContainer& servers, NodeContainer& clients, uint32_t packetSize, uint32_t queueSize)
{
  //Create and install the applications on the server and client
  NS_ASSERT(servers.GetN() == clients.GetN());

  static int portNum = 4000;

  for(int i = 0; i < clients.GetN();i++){
    // Create application
    // Interface number of Ipv4 interface = 1 (0 is 0.0.0.0?); addressIndex = 0
    Ptr<Ipv4> ipv4 = clients.Get(i)->GetObject<Ipv4>(); // Interface number of Ipv4 interface = 1 (0 is 0.0.0.0?); addressIndex = 0
    Ipv4InterfaceAddress iaddr = ipv4->GetAddress(1,0);
    Ipv4Address addr = iaddr.GetLocal();
    Address remoteAddress(InetSocketAddress(addr, portNum));
    FileTransferHelper fileHelper(remoteAddress);
    fileHelper.SetAttribute("Protocol", TypeIdValue(MpTcpSocketFactory::GetTypeId()));
    fileHelper.SetAttribute("FileSize", UintegerValue(2*10e6)); // The setting of FileSize should be careful, flowmonitor may fail to trace if too small

    // Install on server
    ApplicationContainer apps = fileHelper.Install(servers.Get(i));

    // Install on client
    PacketSinkHelper packetSink("ns3::TcpSocketFactory", remoteAddress);
    packetSink.Install(clients.Get(i));

    portNum++;
  }
}

};
