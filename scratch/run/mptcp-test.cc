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

namespace ns3{
  std::string g_link_a_BW;
  std::string g_link_b_BW;
  std::string g_link_c_BW;
  std::string g_link_a_delay;
  std::string g_link_b_delay;
  std::string g_link_c_delay;
  double g_link_b_BER;
  uint32_t g_tcp_buffer_size;
  uint32_t g_router_b_buffer_size;
  uint32_t g_router_c_buffer_size;
  uint32_t g_topology_id;
};

int main(int argc, char* argv[])
{
  enum AppType
  {
    onoff = 0,
    filetransfer = 1
  };

  uint32_t type = 0;
  uint32_t appType = 0;
  std::string outputDir = "mptcp_output";
  uint32_t simulationDuration = 180;

  CommandLine cmd;
  std::string link_b_BER;
  std::string tcp_buffer_size;
  std::string router_b_buffer_size;
  std::string router_c_buffer_size;
  std::string topology_id;
  cmd.AddValue("appType", "The type of application to run", appType);
  cmd.AddValue("outputDir", "The output directory to write the logs to.", outputDir);
  cmd.AddValue("link_a_BW", "Bandwidth of link A.", g_link_a_BW);
  cmd.AddValue("link_b_BW", "Bandwidth of link B.", g_link_b_BW);
  cmd.AddValue("link_c_BW", "Bandwidth of link C.", g_link_c_BW);
  cmd.AddValue("link_a_delay", "Bandwidth of link A.", g_link_a_delay);
  cmd.AddValue("link_b_delay", "Bandwidth of link B.", g_link_b_delay);
  cmd.AddValue("link_c_delay", "Bandwidth of link C.", g_link_c_delay);
  cmd.AddValue("link_b_BER", "The bit error rate of link B.", link_b_BER);
  cmd.AddValue("tcp_buffer_size", "The buffer size of tcp and meta-socket", tcp_buffer_size); // settings in SetConfigDefaults, unit is byte
  cmd.AddValue("router_b_buffer_size", "The buffer size of router queue", router_b_buffer_size); // settings in mptcp-helper-topology.cc, unit is packet count
  cmd.AddValue("router_c_buffer_size", "The buffer size of router queue", router_c_buffer_size); // settings in mptcp-helper-topology.cc, unit is packet count
  cmd.AddValue("topology_id", "The id of topology to use", topology_id);
  cmd.Parse(argc, argv);
  g_link_b_BER = std::stod(link_b_BER);
  g_tcp_buffer_size = uint32_t(std::stoi(tcp_buffer_size));
  g_router_b_buffer_size = uint32_t(std::stoi(router_b_buffer_size));
  g_router_c_buffer_size = uint32_t(std::stoi(router_c_buffer_size));
  g_topology_id = uint32_t(std::stoi(topology_id));
  std::cout << "Please confirm argv: " << g_link_a_BW << " " << g_link_b_BW << " " << g_link_c_BW << " " << link_b_BER << " "
            << g_tcp_buffer_size << " " << g_router_b_buffer_size << " " << g_router_c_buffer_size << std::endl;

  string linkRate = "10Mbps"; // not used
  string linkDelay = "10ms"; // not used

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

  NodeContainer server;
  NodeContainer client;
  NodeContainer middle;
  NodeContainer other_servers;
  NodeContainer other_clients;
  vector<Ptr<NetDevice>> unstableDevices;

  // unstableDevices = CreateSimplestNetwork(segmentSizeWithoutHeaders, server, client, middle, other_servers, other_clients);
  switch(g_topology_id){
    case 0:{
        unstableDevices = CreateNetwork5(segmentSizeWithoutHeaders, server, client, middle, other_servers, other_clients);
        break;
      }

    default:{
      NS_FATAL_ERROR("Unknown topology!");
      break;
      }
  }

// Select one of the following 4 topology of network
CreateSimplestNetworkWithOtherTraffic(segmentSizeWithoutHeaders, server, client, middle, other_servers, other_clients);
// CreateClassicNetwork(segmentSizeWithoutHeaders, server, client, middle, other_servers, other_clients);
// CreateExtendedClassicNetwork (segmentSizeWithoutHeaders, server, client, middle, other_servers, other_clients);
// CreateClassicNetworkWithOtherTraffic (segmentSizeWithoutHeaders, server, client, middle, other_servers, other_clients);

  ConfigureTracing(outputDir, server, client, middle, other_servers, other_clients);

  //Create and install the applications on the server and client
  if(appType == onoff)
  {
    std::cout << "Application type: onoff\n";
    InstallOnOffApplications(server, client, segmentSizeWithoutHeaders);
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

  // AnimationInterface anim ("mptcp-animation.xml");
  // Simulator::Schedule(Seconds(8), &PrintMonitorStates);

  for(int i = 0; i < simulationDuration * 10;i++){
    Simulator::Schedule(Seconds(i/10.0), &TraceMonitorStates, outputDir);
  }

  for(auto dev:unstableDevices){
    Simulator::Schedule(Seconds (30.0), ChangeLinkErrorRate, dev, 1.0);
  }

  for(auto dev:unstableDevices){
    Simulator::Schedule(Seconds (40.0), ChangeLinkErrorRate, dev, 0);
  }

  Simulator::Stop (Seconds(simulationDuration));

  // Print out nodes' interfaces info
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

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
