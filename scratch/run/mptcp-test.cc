#include <string>
#include "mptcp-helper-system.h"
#include "mptcp-helper-router.h"
#include "mptcp-helper-trace.h"
#include "mptcp-helper-topology.h"
#include "mptcp-helper-application.h"

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

  CommandLine cmd;
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

  SetConfigDefaults(linkRate, linkDelay, segmentSize, segmentSizeWithoutHeaders, queueSize);

  //Create the nodes in the topology, and install the internet stack on them
  NodeContainer server;
  NodeContainer client;
  NodeContainer isps;
  NodeContainer ixs;    // Internet exchange points

  Ipv4Address remoteClient;

  CreateRealNetwork (segmentSizeWithoutHeaders, server, client, isps, ixs, remoteClient);

  //Create and install the applications on the server and client
  if(appType == onoff)
  {
    std::cout << "Application type: onoff\n";
    InstallOnOffApplications(server, client, remoteClient, segmentSizeWithoutHeaders);
  }
  else if (appType == filetransfer)
  {
    std::cout << "Application type: filetransfer\n";
    InstallFileTransferApplications(server, client, remoteClient,
                                    segmentSizeWithoutHeaders,
                                    queueSize);
  }

  //Populate and print the IP routing tables
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  Ipv4GlobalRoutingHelper g;
  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("dynamic-global-routing-mptcp.routes", std::ios::out);
  g.PrintRoutingTableAllAt (Seconds (0.5), routingStream);

  //Create an output directory and configure tracing
  ConfigureTracing(outputDir, server, client, isps, ixs);
  AnimationInterface anim ("mptcp-animation.xml");

  // Connect to machine learning process
  // rl::InterfaceToRL rl_socket("127.0.0.1", 12345);
  // rl_socket << "time$12323#cwnd$1...and others\n";
  // string test_str;
  // rl_socket >> test_str;
  // cout << test_str;
  //Set the simulator stop time
  // Simulator::Schedule(Seconds(1), &GetThroughout);
  // Simulator::Schedule(Seconds(2), &GetThroughout);
  // Simulator::Schedule(Seconds(3), &GetThroughout);
  // Simulator::Schedule(Seconds(4), &GetThroughout);
  // Simulator::Schedule(Seconds(5), &GetThroughout);
  Simulator::Stop (Seconds(30.0));
  // GetThroughout();

  //Begin the simulation
  Simulator::Run ();

  Simulator::Destroy ();
  return 0;
}
