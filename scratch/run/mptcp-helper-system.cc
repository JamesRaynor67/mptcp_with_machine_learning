#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include <sys/stat.h>
#include <unistd.h>

namespace ns3{
extern uint32_t g_tcp_buffer_size;

void CheckAndCreateDirectory(std::string path)
{
  if(access(path.c_str(), F_OK ) == -1 ){
    const int error = mkdir(path.c_str(), S_IRWXU | S_IRWXG |  S_IROTH);

    if(error == -1){
      NS_FATAL_ERROR("Could not create directory " << path);
    }
  }
}

void SetConfigDefaults (std::string linkRate, std::string linkDelay,
                        uint32_t segmentSize, uint32_t segmentSizeWithoutHeaders,
                        uint32_t queueSize)
{
  //The bandwidth delay product
  //uint32_t bdp = DataRate(linkRate).GetBitRate() * Time(linkDelay).GetSeconds() * 4;
  //uint32_t bdpBytes = bdp/8;

  //TCP configuration
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (segmentSizeWithoutHeaders));

  //Slow start threshold
  Config::SetDefault("ns3::TcpSocket::InitialSlowStartThreshold", UintegerValue(numeric_limits<uint32_t>::max()));

  //Set the receive window size to the maximum possible
  Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue(1<<30));

  //I need timestap to be enabled to calculate RTT correctly
  Config::SetDefault("ns3::TcpSocketImpl::Timestamp", BooleanValue(true));

  //Set the mptcp option
  Config::SetDefault("ns3::TcpSocketImpl::EnableMpTcp", BooleanValue(true));

  // Config::SetDefault ("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents", BooleanValue (true));

  //Set the initial congestion window to be larger than duplicate ack threshold
  Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue(4));

  //Config::SetDefault("ns3::Queue::Mode", EnumValue(Queue::QUEUE_MODE_BYTES));
  //Config::SetDefault("ns3::Queue::MaxBytes", UintegerValue(queueSize));

  Config::SetDefault("ns3::RedQueueDisc::Mode", EnumValue(Queue::QUEUE_MODE_BYTES));
  Config::SetDefault("ns3::RedQueueDisc::MinTh", DoubleValue(queueSize * 0.33));
  Config::SetDefault("ns3::RedQueueDisc::MaxTh", DoubleValue(queueSize * 0.66));
  Config::SetDefault("ns3::RedQueueDisc::QueueLimit", UintegerValue(queueSize));
  Config::SetDefault("ns3::RedQueueDisc::QW", DoubleValue(1));
  Config::SetDefault("ns3::RedQueueDisc::LInterm", DoubleValue(10));
  Config::SetDefault("ns3::RedQueueDisc::Wait", BooleanValue(false));

  //Set the send buffer to be the n * queue size to accomodate n subflows
  //Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (interfaces * queueSize));
  // Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (numeric_limits<uint32_t>::max()));
  // Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (256*1024)); // in linux, default is 256K, max is 8M, recommendation is 2 * delay * bandwidth
  // Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (8*1024*1024)); // in linux, default is 256K, max is 8M, recommendation is 2 * delay * bandwidth
  Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (g_tcp_buffer_size)); // in linux, default is 256K, max is 8M, recommendation is 2 * delay * bandwidth

  Config::SetDefault ("ns3::TcpSocket::ConnTimeout", TimeValue(Seconds(2.0)));
  Config::SetDefault("ns3::ArpCache::AliveTimeout", TimeValue(Seconds(120 + 1)));

  Config::SetDefault("ns3::MpTcpMetaSocket::TagSubflows", BooleanValue(true));

  //Config::SetDefault("ns3::Ipv4::WeakEsModel", BooleanValue(false));
}

void EnableLogging ()
{
  // LogComponentEnable("Ipv4StaticRouting", LOG_LEVEL_ALL);
  // LogComponentEnable("Ipv4GlobalRouting", LOG_LEVEL_ALL);
  // LogComponentEnable("TcpL4Protocol", LOG_LEVEL_ALL);
  // LogComponentEnable("TcpSocketBase", LOG_LEVEL_ALL);
  // LogComponentEnable("MpTcpMetaSocket", LOG_LEVEL_ALL);
  // LogComponentEnable("MpTcpSubflow", LOG_LEVEL_ALL);
  // LogComponentEnable("MpOnOffApplication", LOG_LEVEL_ALL);
  // LogComponentEnable("Ipv4EndPointDemux", LOG_LEVEL_ALL);
  // // //
  // LogComponentEnable ("Ipv4L3Protocol", LOG_LEVEL_ALL);
  // LogComponentEnable("PointToPointNetDevice", LOG_LEVEL_ALL);
  // LogComponentEnable ("Ipv4EndPoint", LOG_LEVEL_ALL);
}

};
