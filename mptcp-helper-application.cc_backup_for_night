#include "mp-onoff-application.h"

#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/random-variable-stream.h"
#include "file-transfer-helper.h"
#include "file-transfer-application.h"

namespace ns3{

void InstallOnOffApplications(NodeContainer& servers, NodeContainer& clients, uint32_t packetSize)
{
  // Create and install the applications on nodes
  // The first pair will be installed with MPTCP, and the rest will be installed with TCP
  static int mptcpAppNum = 1;
  NS_ASSERT(servers.GetN() == clients.GetN());

  static int sinkPort = 4000;
  for(int i = 0; i < servers.GetN();i++){
    Ptr<Ipv4> ipv4 = servers.Get(i)->GetObject<Ipv4>(); // Interface number of Ipv4 interface = 1 (0 is 0.0.0.0?); addressIndex = 0
    Ipv4InterfaceAddress iaddr = ipv4->GetAddress(1,0);
    Ipv4Address addr = iaddr.GetLocal();
    Address sinkAddress(InetSocketAddress(addr, sinkPort));
    std::cout << "From: " << clients.Get(i)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal() << " To: " << addr << endl;

    // Create application
    if(mptcpAppNum > 0){
      mptcpAppNum--;
      std::cout << "MPTCP OnOff App installed" << std::endl;
      Ptr<Application> onOff;
      Ptr<MpOnOffApplication> MpOnOff = CreateObject<MpOnOffApplication>();
      MpOnOff->SetAttribute("Protocol", StringValue("ns3::MpTcpSocketFactory"));
      MpOnOff->SetAttribute("Remote", AddressValue (sinkAddress));
      MpOnOff->SetAttribute("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
      MpOnOff->SetAttribute("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));
      MpOnOff->SetAttribute("DataRate", DataRateValue (DataRate("0.3Mbps")));
      MpOnOff->SetAttribute("PacketSize", UintegerValue (packetSize));
      onOff = ns3::DynamicCast<Application>(MpOnOff);

      // Install on server
      clients.Get(i)->AddApplication(onOff);

      // Install on client
      PacketSinkHelper packetSink("ns3::MpTcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
      // PacketSinkHelper packetSink("ns3::MpTcpSocketFactory", sinkAddress);
      packetSink.Install(servers.Get(i));
      sinkPort++;
    }
    else{
      std::cout << "TCP OnOff App installed" << std::endl;
      // PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
      PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
      ApplicationContainer sinkApps = packetSinkHelper.Install(servers.Get(i));
      // sinkApps.Start (Seconds (0.));
      // sinkApps.Stop (Seconds (60.));

      OnOffHelper onOffHelper ("ns3::TcpSocketFactory", sinkAddress);
      // OnOffHelper onOffHelper ("ns3::UdpSocketFactory", sinkAddress);
      onOffHelper.SetAttribute ("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=30]"));
      onOffHelper.SetAttribute ("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=30]"));
      // onOffHelper.SetAttribute ("OnTime", StringValue("ns3::UniformRandomVariable[Min=0|Max=5]"));
      // onOffHelper.SetAttribute ("OffTime", StringValue("ns3::UniformRandomVariable[Min=0|Max=5]"));
      onOffHelper.SetAttribute ("DataRate",StringValue ("0.05Mbps"));
      onOffHelper.SetAttribute ("PacketSize", UintegerValue (packetSize));

      ApplicationContainer source;

      source.Add (onOffHelper.Install (clients.Get(i)));
      // source.Start (Seconds (1.1));
      // source.Stop (Seconds (60.0));

// ///////////////////////////////
//
//       std::cout << "TCP OnOff App installed" << std::endl;
//       std::cout << "1" << std::endl;
//       onOff->SetAttribute("Protocol", StringValue("ns3::TcpSocketFactory"));
//       // onOff->SetAttribute("Remote", AddressValue (remoteAddress));
//       std::cout << "2" << std::endl;
//       onOff->SetAttribute("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
//       std::cout << "2.1" << std::endl;
//       onOff->SetAttribute("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));
//       std::cout << "2.2" << std::endl;
//       onOff->SetAttribute("DataRate", DataRateValue (DataRate("0.1Mbps")));
//       onOff->SetAttribute("PacketSize", UintegerValue (packetSize));
//       std::cout << "3" << std::endl;
//       // Install on server
//       servers.Get(i)->AddApplication(onOff);
//       std::cout << "4" << std::endl;
//
//       // Install on client
//       PacketSinkHelper packetSink("ns3::TcpSocketFactory", remoteAddress);
//       std::cout << "5" << std::endl;
//       packetSink.Install(clients.Get(i));
//       std::cout << "6" << std::endl;
//       sinkPort++;
//       std::cout << "TCP OnOff App installed, over" << std::endl;
    }
  }
}

void InstallFileTransferApplications(NodeContainer& servers, NodeContainer& clients, uint32_t packetSize, uint32_t queueSize)
{
  //Create and install the applications on the server and client
  NS_ASSERT(servers.GetN() == clients.GetN());

  static int portNum = 4000;
  std::cout << "WTF InstallFileTransferApplications 0" << endl;

  for(int i = 0; i < clients.GetN();i++){
    // Create application
    // Interface number of Ipv4 interface = 1 (0 is 0.0.0.0?); addressIndex = 0
    std::cout << "InstallFileTransferApplications 1" << endl;
    Ptr<Ipv4> ipv4 = servers.Get(i)->GetObject<Ipv4>(); // Interface number of Ipv4 interface = 1 (0 is 0.0.0.0?); addressIndex = 0
    Ipv4InterfaceAddress iaddr = ipv4->GetAddress(1,0);
    Ipv4Address addr = iaddr.GetLocal();
    Address remoteAddress(InetSocketAddress(addr, portNum));
    FileTransferHelper fileHelper(remoteAddress);
    // fileHelper.SetAttribute("Protocol", TypeIdValue(TcpSocketFactory::GetTypeId()));
    fileHelper.SetAttribute("Protocol", TypeIdValue(MpTcpSocketFactory::GetTypeId()));
    fileHelper.SetAttribute("FileSize", UintegerValue(6.75*1e6)); // The setting of FileSize should be careful, flowmonitor may fail to trace if too small

    // Install on server
    ApplicationContainer apps = fileHelper.Install(clients.Get(i));

    // Install on client
    // PacketSinkHelper packetSink("ns3::TcpSocketFactory", remoteAddress);
    PacketSinkHelper packetSink("ns3::MpTcpSocketFactory", remoteAddress);
    packetSink.Install(servers.Get(i));

    portNum++;
  }
}

};
