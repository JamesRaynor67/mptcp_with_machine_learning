#pragma once

#include "ns3/internet-module.h"

namespace ns3{

void TraceMacRx(Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet);

void TraceMacTx(Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet);

void TraceQueueItemDrop(Ptr<OutputStreamWrapper> stream, Ptr<const QueueItem> item);

void ConfigureTracing (const string& outputDir,
                       const NodeContainer& clients,
                       const NodeContainer& switches,
                       const NodeContainer& servers);
}
