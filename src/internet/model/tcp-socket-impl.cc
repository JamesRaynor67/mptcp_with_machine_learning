//
//  tcp-socket-impl.cc
//  ns3
//
//  Created by Lynne Salameh on 23/7/16.
//  Copyright © 2016 University College London. All rights reserved.
//

#include "tcp-socket-impl.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/pointer.h"
#include "ns3/node.h"
#include "ns3/log.h"
#include "tcp-congestion-ops.h"
#include "ns3/packet.h"

namespace ns3
{
  
NS_LOG_COMPONENT_DEFINE("TcpSocketImpl");

NS_OBJECT_ENSURE_REGISTERED(TcpSocketImpl);
 
TcpSocketImpl::TcpSocketImpl () : TcpSocket ()
                                  , m_node (0)
                                  , m_tcp (0)
                                  , m_rtt (0)
                                  , m_congestionControl (0)
{
  m_tcpParams = Create<TcpParameters>();
}

TcpSocketImpl::TcpSocketImpl(const TcpSocketImpl& sock) : TcpSocket (sock)
                                                        , m_node (sock.m_node)
                                                        , m_tcp (sock.m_tcp)
                                                        , m_rtt (0)
                                                        , m_congestionControl (0)
{
  //Deep copy
  m_tcpParams = Copy<TcpParameters>(sock.m_tcpParams);
  
  // Copy the rtt estimator if it is set
  if (sock.m_rtt)
  {
    m_rtt = sock.m_rtt->Copy ();
  }
  
  if (sock.m_congestionControl)
  {
    m_congestionControl = sock.m_congestionControl->Fork ();
  }
}
  
TcpSocketImpl::~TcpSocketImpl ()
{
}
  
TypeId
TcpSocketImpl::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::TcpSocketImpl")
  .SetParent<TcpSocket>()
  //From Tcp Socket Base
  .AddAttribute ("MaxSegLifetime",
                 "Maximum segment lifetime in seconds, use for TIME_WAIT state transition to CLOSED state",
                 DoubleValue (120), /* RFC793 says MSL=2 minutes*/
                 MakeDoubleAccessor (&TcpSocketImpl::SetMaxSegLifetime,
                                     &TcpSocketImpl::GetMaxSegLifetime),
                 MakeDoubleChecker<double> (0))
  .AddAttribute ("MaxWindowSize", "Max size of advertised window",
                 UintegerValue (65535),
                 MakeUintegerAccessor (&TcpSocketImpl::SetMaxWindowSize,
                                       &TcpSocketImpl::GetMaxWindowSize),
                 MakeUintegerChecker<uint16_t> ())
  .AddAttribute ("NullISN",
                 "Set to false if you want to enable random isn",
                 BooleanValue (true),
                 MakeBooleanAccessor (&TcpSocketImpl::SetNullIsn,
                                      &TcpSocketImpl::GetNullIsn),
                 MakeBooleanChecker ())
  .AddAttribute ("EnableMpTcp",
                 "Enable or disable MPTCP support",
                 BooleanValue (false),
                 MakeBooleanAccessor (&TcpSocketImpl::SetMptcpEnabled,
                                      &TcpSocketImpl::GetMptcpEnabled),
                 MakeBooleanChecker ())
  .AddAttribute ("WindowScaling", "Enable or disable Window Scaling option",
                 BooleanValue (true),
                 MakeBooleanAccessor (&TcpSocketImpl::SetWindowScalingEnabled,
                                      &TcpSocketImpl::GetWindowScalingEnabled),
                 MakeBooleanChecker ())
  .AddAttribute ("Timestamp", "Enable or disable Timestamp option",
                 BooleanValue (true),
                 MakeBooleanAccessor (&TcpSocketImpl::SetTimestampEnabled,
                                      &TcpSocketImpl::GetTimestampEnabled),
                 MakeBooleanChecker ())
  .AddAttribute ("MinRto",
                 "Minimum retransmit timeout value",
                 TimeValue (Seconds (1.0)), // RFC 6298 says min RTO=1 sec, but Linux uses 200ms. See http://www.postel.org/pipermail/end2end-interest/2004-November/004402.html
                 MakeTimeAccessor (&TcpSocketImpl::SetMinRto,
                                   &TcpSocketImpl::GetMinRto),
                 MakeTimeChecker ())
  .AddAttribute ("ClockGranularity",
                 "Clock Granularity used in RTO calculations",
                 TimeValue (MilliSeconds (1)), // RFC6298 suggest to use fine clock granularity
                 MakeTimeAccessor (&TcpSocketImpl::SetClockGranularity,
                                   &TcpSocketImpl::GetClockGranularity),
                 MakeTimeChecker ())
  .AddAttribute ("ReTxThreshold", "Threshold for fast retransmit",
                 UintegerValue (3),
                 MakeUintegerAccessor (&TcpSocketImpl::SetReTxThreshold,
                                       &TcpSocketImpl::GetReTxThreshold),
                 MakeUintegerChecker<uint32_t> ())
  .AddAttribute ("LimitedTransmit", "Enable limited transmit",
                 BooleanValue (false),
                 MakeBooleanAccessor (&TcpSocketImpl::SetLimitedTransmit,
                                      &TcpSocketImpl::GetLimitedTransmit),
                 MakeBooleanChecker ())
  
  ;
  return tid;
}
  
TypeId TcpSocketImpl::GetInstanceTypeId(void) const
{
  return TcpSocketImpl::GetTypeId();
}
  
Ptr<const TcpParameters> TcpSocketImpl::GetTcpParameters () const
{
  return m_tcpParams;
}

void TcpSocketImpl::SetTcpParameters (Ptr<const TcpParameters> params)
{
  //Deep copy
  m_tcpParams = Copy<TcpParameters>(params);
}
  
/* Below are the attribute get/set functions */

void TcpSocketImpl::SetConnTimeout (Time timeout)
{
  NS_LOG_FUNCTION (this << timeout);
  m_tcpParams->m_cnTimeout = timeout;
}

Time TcpSocketImpl::GetConnTimeout (void) const
{
  return m_tcpParams->m_cnTimeout;
}

void TcpSocketImpl::SetSynRetries (uint32_t count)
{
  NS_LOG_FUNCTION (this << count);
  m_tcpParams->m_synRetries = count;
}

uint32_t TcpSocketImpl::GetSynRetries (void) const
{
  return m_tcpParams->m_synRetries;
}

void TcpSocketImpl::SetDataRetries (uint32_t retries)
{
  NS_LOG_FUNCTION (this << retries);
  m_tcpParams->m_dataRetries = retries;
}

uint32_t TcpSocketImpl::GetDataRetries (void) const
{
  NS_LOG_FUNCTION (this);
  return m_tcpParams->m_dataRetries;
}

void TcpSocketImpl::SetDelAckTimeout (Time timeout)
{
  NS_LOG_FUNCTION (this << timeout);
  m_tcpParams->m_delAckTimeout = timeout;
}

Time TcpSocketImpl::GetDelAckTimeout (void) const
{
  return m_tcpParams->m_delAckTimeout;
}

void
TcpSocketImpl::SetDelAckMaxCount (uint32_t count)
{
  NS_LOG_FUNCTION (this << count);
  m_tcpParams->m_delAckMaxCount = count;
}

uint32_t
TcpSocketImpl::GetDelAckMaxCount (void) const
{
  return m_tcpParams->m_delAckMaxCount;
}

void
TcpSocketImpl::SetTcpNoDelay (bool noDelay)
{
  NS_LOG_FUNCTION (this << noDelay);
  m_tcpParams->m_noDelay = noDelay;
}

bool
TcpSocketImpl::GetTcpNoDelay (void) const
{
  return m_tcpParams->m_noDelay;
}

void
TcpSocketImpl::SetPersistTimeout (Time timeout)
{
  NS_LOG_FUNCTION (this << timeout);
  m_tcpParams->m_persistTimeout = timeout;
}

Time
TcpSocketImpl::GetPersistTimeout (void) const
{
  return m_tcpParams->m_persistTimeout;
}

bool TcpSocketImpl::SetAllowBroadcast (bool allowBroadcast)
{
  // Broadcast is not implemented. Return true only if allowBroadcast==false
  return (!allowBroadcast);
}

bool TcpSocketImpl::GetAllowBroadcast (void) const
{
  return false;
}

/* Associate a node with this TCP socket */
void TcpSocketImpl::SetNode (Ptr<Node> node)
{
  m_node = node;
}
  
Ptr<Node> TcpSocketImpl::GetNode (void) const
{
  return m_node;
}
  
/* Inherit from Socket class: Returns socket type, NS3_SOCK_STREAM */
enum Socket::SocketType
TcpSocketImpl::GetSocketType (void) const
{
  return NS3_SOCK_STREAM;
}

/* Associate the L4 protocol (e.g. mux/demux) with this socket */
void TcpSocketImpl::SetTcp (Ptr<TcpL4Protocol> tcp)
{
  m_tcp = tcp;
}
  
void TcpSocketImpl::SetCongestionControlAlgorithm (Ptr<TcpCongestionOps> algo)
{
  NS_LOG_FUNCTION (this << algo);
  m_congestionControl = algo;
}

  
void TcpSocketImpl::SetMaxSegLifetime (double msl)
{
  m_tcpParams->m_msl = msl;
}
  
double TcpSocketImpl::GetMaxSegLifetime () const
{
  return m_tcpParams->m_msl;
}

void TcpSocketImpl::SetMaxWindowSize (uint16_t size)
{
  m_tcpParams->m_maxWinSize = size;
}
  
uint16_t TcpSocketImpl::GetMaxWindowSize () const
{
  return m_tcpParams->m_maxWinSize;
}

void TcpSocketImpl::SetNullIsn (bool flag)
{
  m_tcpParams->m_nullIsn = flag;
}
  
bool TcpSocketImpl::GetNullIsn () const
{
  return m_tcpParams->m_nullIsn;
}

void TcpSocketImpl::SetMptcpEnabled (bool flag)
{
  m_tcpParams->m_mptcpEnabled = flag;
}
  
bool TcpSocketImpl::GetMptcpEnabled () const
{
  return m_tcpParams->m_mptcpEnabled;
}

void TcpSocketImpl::SetWindowScalingEnabled (bool flag)
{
  m_tcpParams->m_winScalingEnabled = flag;
}

bool TcpSocketImpl::GetWindowScalingEnabled () const
{
  return m_tcpParams->m_winScalingEnabled;
}

void TcpSocketImpl::SetTimestampEnabled (bool flag)
{
  m_tcpParams->m_timestampEnabled = flag;
}

bool TcpSocketImpl::GetTimestampEnabled () const
{
  return m_tcpParams->m_timestampEnabled;
}

void TcpSocketImpl::SetReTxThreshold (uint32_t thresh)
{
  m_tcpParams->m_retxThresh = thresh;
}

uint32_t TcpSocketImpl::GetReTxThreshold () const
{
  return m_tcpParams->m_retxThresh;
}


void TcpSocketImpl::SetLimitedTransmit (bool flag)
{
  m_tcpParams->m_limitedTx = flag;
}

bool TcpSocketImpl::GetLimitedTransmit () const
{
  return m_tcpParams->m_limitedTx;
}
  
void TcpSocketImpl::SetMinRto (Time minRto)
{
  m_tcpParams->m_minRto = minRto;
}

Time TcpSocketImpl::GetMinRto (void) const
{
  return m_tcpParams->m_minRto;
}

void TcpSocketImpl::SetClockGranularity (Time clockGranularity)
{
  m_tcpParams->m_clockGranularity = clockGranularity;
}

Time TcpSocketImpl::GetClockGranularity (void) const
{
  return m_tcpParams->m_clockGranularity;
}
  
Ptr<const RttEstimator> TcpSocketImpl::GetRttEstimator()
{
  //!
  return m_rtt;
}
  
/* Set an RTT estimator with this socket */
void TcpSocketImpl::SetRtt (Ptr<RttEstimator> rtt)
{
  m_rtt = rtt;
}

}
