namespace ns3{

Ptr<Ipv4StaticRouting> GetNodeStaticRoutingProtocol(Ptr<Node> node);

void PopulateServerRoutingTable(Ptr<Node> aServer,
                                Ipv4InterfaceContainer& clientInterfaces,
                                Ipv4InterfaceContainer& switchClientInterfaces,
                                Ipv4InterfaceContainer& switchServerInterfaces);

void PopulateSwitchRoutingTable(Ptr<Node> aSwitch,
                                Ipv4InterfaceContainer& clientInterfaces,
                                Ipv4InterfaceContainer& serverInterfaces,
                                Ipv4InterfaceContainer& switchClientInterfaces,
                                Ipv4InterfaceContainer& switchServerInterfaces);

void PopulateClientRoutingTable(Ptr<Node> aClient,
                                Ipv4InterfaceContainer& serverInterfaces,
                                Ipv4InterfaceContainer& switchClientInterfaces,
                                Ipv4InterfaceContainer& switchServerInterfaces);

void PrintRoutingTable(Ptr<Node> aNode,
                       const string& outputDir,
                       const string& filePrefix);

};
