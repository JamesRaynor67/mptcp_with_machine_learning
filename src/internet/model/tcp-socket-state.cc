//
//  mptcp-socket-state.cc
//  ns3
//
//  Created by Lynne Salameh on 24/7/16.
//  Copyright © 2016 University College London. All rights reserved.
//

#include "tcp-socket-state.h"
#include "tcp-socket-impl.h"

namespace ns3
{
  
  NS_OBJECT_ENSURE_REGISTERED(TcpSocketState);
  
  TypeId
  TcpSocketState::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::TcpSocketState")
    .SetParent<Object> ()
    .SetGroupName ("Internet")
    .AddConstructor <TcpSocketState> ()
    .AddTraceSource ("CongestionWindow",
                     "The TCP connection's congestion window",
                     MakeTraceSourceAccessor (&TcpSocketState::m_cWnd),
                     "ns3::TracedValue::Uint32Callback")
    .AddTraceSource ("SlowStartThreshold",
                     "TCP slow start threshold (bytes)",
                     MakeTraceSourceAccessor (&TcpSocketState::m_ssThresh),
                     "ns3::TracedValue::Uint32Callback")
    .AddTraceSource ("CongState",
                     "TCP Congestion machine state",
                     MakeTraceSourceAccessor (&TcpSocketState::m_congState),
                     "ns3::TracedValue::TcpOldCongStatesTracedValueCallback")
    .AddTraceSource ("HighestSequence",
                     "Highest sequence number received from peer",
                     MakeTraceSourceAccessor (&TcpSocketState::m_highTxMark),
                     "ns3::SequenceNumber32TracedValueCallback")
    .AddTraceSource ("NextTxSequence",
                     "Next sequence number to send (SND.NXT)",
                     MakeTraceSourceAccessor (&TcpSocketState::m_nextTxSequence),
                     "ns3::SequenceNumber32TracedValueCallback")
    ;
    return tid;
  }
  
  TcpSocketState::TcpSocketState(void)
  : Object (),
  m_cWnd (0),
  m_ssThresh (0),
  m_initialCWnd (0),
  m_initialSsThresh (0),
  m_segmentSize (0),
  m_lastAckedSeq (0),
  m_socket (0),
  m_congState (CA_OPEN),
  m_highTxMark (0),
  // Change m_nextTxSequence for non-zero initial sequence number
  m_nextTxSequence (0)
  {
  }
  
  TcpSocketState::TcpSocketState (const TcpSocketState &other)
  : Object (other),
  m_cWnd (other.m_cWnd),
  m_ssThresh (other.m_ssThresh),
  m_initialCWnd (other.m_initialCWnd),
  m_initialSsThresh (other.m_initialSsThresh),
  m_segmentSize (other.m_segmentSize),
  m_socket (0), 
  m_lastAckedSeq (other.m_lastAckedSeq),
  m_congState (other.m_congState),
  m_highTxMark (other.m_highTxMark),
  m_nextTxSequence (other.m_nextTxSequence)
  {
  }
  
  const char* const
  TcpSocketState::TcpCongStateName[TcpSocketState::CA_LAST_STATE] =
  {
    "CA_OPEN", "CA_DISORDER", "CA_CWR", "CA_RECOVERY", "CA_LOSS"
  };
}
