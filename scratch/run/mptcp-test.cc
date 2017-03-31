#include <string>
#include "mptcp-helper-system.h"
#include "mptcp-helper-router.h"
#include "mptcp-helper-trace.h"
#include "mptcp-helper-topology.h"
#include "mptcp-helper-application.h"

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"

using namespace ns3;

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

  switch(type){
    case (bottleneck):
    {

      /*
       * Server has two possible paths to client, there is a bottleneck link at switch.
       *
       ------           ------           ------
      |      | ------  |      |-------  |      |
      |      | ------  |      |         |      |
       ------           ------           ------
       Server           Switch           Client
       *
       */
      CreateMultipleFlowsSingleBottleneck(interfaceCount, segmentSizeWithoutHeaders,
                                          rate, delay,
                                          servers, switches, clients,
                                          remoteClient);
      break;
    }
    case (nobottleneck):
    {

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
      break;
    }
    case (otherflow):
    {
      /*
       * 2 MPTCP flows share a bottleneck with a normal TCP flow.
       *
       ------           ------           ------
      |      | ------  |      |-------  |      |
      |      | ------  |      |         |      |
       ------          |      |          ------
       MPTCP           |      |          Client
       Server          |      |
                       |      |
       ------          |      |
      |      | ------  |      |
      |      |         |      |
       ------           ------
       Server2            Switch
       */

      CreateMultipleAndTcpFlows (interfaceCount, segmentSizeWithoutHeaders,
                                 rate, delay,
                                 servers, switches, clients,
                                 remoteClient);
      break;
    }
    default:
    {
      CreateMultipleFlowsSingleBottleneck(interfaceCount, segmentSizeWithoutHeaders,
                                          rate, delay,
                                          servers, switches, clients,
                                          remoteClient);
    }
  }

  //Create and install the applications on the server and client
  if(appType == onoff)
  {
    InstallOnOffApplications(servers, clients, remoteClient, segmentSizeWithoutHeaders);
  }
  else if (appType == filetransfer)
  {
    InstallFileTransferApplications(servers, clients, remoteClient,
                                    segmentSizeWithoutHeaders,
                                    queueSize);
  }

  //Populate the IP routing tables
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  //Create an output directory and configure tracing
  ConfigureTracing(outputDir, clients, switches, servers);

  //Set the simulator stop time
  Simulator::Stop (Seconds(20.0));

  //Begin the simulation
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
