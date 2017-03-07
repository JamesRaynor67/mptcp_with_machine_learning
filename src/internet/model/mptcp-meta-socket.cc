/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 University of Sussex
 * Copyright (c) 2015 Université Pierre et Marie Curie (UPMC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author:  Matthieu Coudron <matthieu.coudron@lip6.fr>
 *          Morteza Kheirkhah <m.kheirkhah@sussex.ac.uk>
 *          Lynne Salameh <l.salameh@cs.ucl.ac.uk>
 */
#undef NS_LOG_APPEND_CONTEXT
#define NS_LOG_APPEND_CONTEXT \
  if (m_node) { std::clog << Simulator::Now ().GetSeconds () << " [node " << m_node->GetId () << "] "; }

#include <algorithm>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <map>
#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/string.h"
#include "mptcp-meta-socket.h"
#include "tcp-l4-protocol.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/error-model.h"
#include "ns3/pointer.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/object-vector.h"
#include "ns3/mptcp-scheduler-round-robin.h"
#include "ns3/mptcp-id-manager.h"
#include "ns3/mptcp-id-manager-impl.h"
#include "ns3/mptcp-subflow.h"
#include "ns3/tcp-option-mptcp.h"
#include "ns3/callback.h"
#include "ns3/trace-helper.h"
#include "ipv6-end-point.h"
#include "tcp-congestion-ops.h"
#include "mptcp-crypto.h"
#include <limits>


using namespace std;


namespace ns3
{
NS_LOG_COMPONENT_DEFINE("MpTcpMetaSocket");

NS_OBJECT_ENSURE_REGISTERED(MpTcpMetaSocket);

NS_OBJECT_ENSURE_REGISTERED(MpTcpSubflowTag);

/*
 Subflow Tags
 */

MpTcpSubflowTag::MpTcpSubflowTag() :  m_id (0)
  , m_sourceToken (0)
  , m_destToken (0)

{
}

MpTcpSubflowTag::~MpTcpSubflowTag()
{
}

TypeId MpTcpSubflowTag::GetTypeId (void)
{
  static TypeId tid =TypeId("ns3::MpTcpSubflowTag")
  .SetParent<Tag>()
  .AddConstructor<MpTcpSubflowTag>();
  
  return tid;
}

TypeId MpTcpSubflowTag::GetInstanceTypeId (void) const
{
  return MpTcpSubflowTag::GetTypeId();
}

uint32_t MpTcpSubflowTag::GetSerializedSize (void) const
{
  return 3 * sizeof (uint32_t);
}

void MpTcpSubflowTag::Serialize (TagBuffer i) const
{
  i.WriteU32(m_id);
  i.WriteU32(m_sourceToken);
  i.WriteU32(m_destToken);
}

void MpTcpSubflowTag::Deserialize (TagBuffer i)
{
  m_id = i.ReadU32();
  m_sourceToken = i.ReadU32();
  m_destToken = i.ReadU32();
}
void MpTcpSubflowTag::Print (std::ostream &os) const
{
  os << "Subflow tag with id " << m_id << " source token "
  << m_sourceToken << " dest token " << m_destToken;
}

uint32_t MpTcpSubflowTag::GetSubflowId() const
{
  return m_id;
}

void MpTcpSubflowTag::SetSubflowId(uint32_t subflowId)
{
  m_id = subflowId;
}

uint32_t MpTcpSubflowTag::GetSourceToken () const
{
  return m_sourceToken;
}

void MpTcpSubflowTag::SetSourceToken (uint32_t token)
{
  m_sourceToken = token;
}

uint32_t MpTcpSubflowTag::GetDestToken () const
{
  return m_destToken;
}

void MpTcpSubflowTag::SetDestToken (uint32_t token)
{
  m_destToken = token;
}

MpTcpMetaSocket::MpTcpMetaSocket() :  TcpSocketImpl()
                                    , m_remotePathIdManager(0)
                                    , m_master (0)
                                    , m_scheduler (0)
                                    , m_state (MptcpMetaClosed)
                                    , m_localKey(0)
                                    , m_localToken(0)
                                    , m_peerKey(0)
                                    , m_peerToken(0)
                                    , m_doChecksum(false)
                                    , m_receivedDSS(false)
                                    , m_connected (false)
                                    , m_tagSubflows(false)
                                    , m_subflowTypeId(MpTcpSubflow::GetTypeId ())
                                    , m_schedulerTypeId(MpTcpSchedulerRoundRobin::GetTypeId())
                                    , m_rWnd(0)
                                    , m_initialCWnd(0)
                                    , m_initialSsThresh(0)
                                    , m_segmentSize (0)
                                    , m_lastAckedSeq (0)
                                    , m_highTxMark (0)
                                    , m_nextTxSequence (0)
                                    , m_errno (ERROR_NOTERROR)
                                    , m_sendPendingDataEvent ()
                                    , m_retxEvent ()
                                    , m_lastAckEvent()
                                    , m_timeWaitEvent()
{
  NS_LOG_FUNCTION(this);
  
  m_rxBuffer = CreateObject<TcpRxBuffer64>();
  m_txBuffer = CreateObject<TcpTxBuffer64>();
  
  //not considered as an Object
  m_remotePathIdManager = Create<MpTcpPathIdManagerImpl>();
  
  CreateScheduler(m_schedulerTypeId);
  
  m_subflowConnectionSucceeded  = MakeNullCallback<void, Ptr<MpTcpSubflow> >();
  m_subflowConnectionFailure    = MakeNullCallback<void, Ptr<MpTcpSubflow> >();
  m_subflowAdded = MakeNullCallback<void, Ptr<MpTcpSubflow>, bool> ();
  
  m_tcpParams->m_mptcpEnabled = true;
}

MpTcpMetaSocket::MpTcpMetaSocket(const MpTcpMetaSocket& sock) : TcpSocketImpl(sock)
                                                              , m_remotePathIdManager(0)
                                                              , m_master (0)
                                                              , m_scheduler (0)
                                                              , m_state(sock.m_state)
                                                              , m_localKey(0)
                                                              , m_localToken(0)
                                                              , m_peerKey(sock.m_peerKey)
                                                              , m_peerToken(sock.m_peerToken)
                                                              , m_doChecksum(sock.m_doChecksum)
                                                              , m_receivedDSS(false)
                                                              , m_connected (sock.m_connected)
                                                              , m_tagSubflows(sock.m_tagSubflows)
                                                              , m_subflowTypeId(sock.m_subflowTypeId)
                                                              , m_schedulerTypeId(sock.m_schedulerTypeId)
                                                              , m_rWnd(0)
                                                              , m_initialCWnd(sock.m_initialCWnd)
                                                              , m_initialSsThresh(sock.m_initialSsThresh)
                                                              , m_segmentSize (sock.m_segmentSize)
                                                              , m_lastAckedSeq (0)
                                                              , m_highTxMark (0)
                                                              , m_nextTxSequence (0)
                                                              , m_errno (ERROR_NOTERROR)
                                                              , m_subflowConnectionSucceeded(sock.m_subflowConnectionSucceeded)
                                                              , m_subflowConnectionFailure(sock.m_subflowConnectionFailure)
                                                              , m_joinRequest(sock.m_joinRequest)
                                                              , m_subflowConnectionCreated(sock.m_subflowConnectionCreated)
                                                              , m_subflowAdded(sock.m_subflowAdded)
                                                              , m_sendPendingDataEvent ()
                                                              , m_retxEvent ()
                                                              , m_lastAckEvent()
                                                              , m_timeWaitEvent()
{
  NS_LOG_FUNCTION(this);
  NS_LOG_LOGIC ("Invoked the copy constructor");
  
  m_rxBuffer = CopyObject<TcpRxBuffer64>(sock.m_rxBuffer);
  m_txBuffer = CopyObject<TcpTxBuffer64>(sock.m_txBuffer);
  
  //! Scheduler may have some states, thus generate a new one
  m_remotePathIdManager = Create<MpTcpPathIdManagerImpl>();


  CreateScheduler(m_schedulerTypeId);

  //Generate a new connection key for the copied meta socket.
  GenerateUniqueMpTcpKey();
  
  // Reset all callbacks to null
  Callback<void, Ptr<MpTcpMetaSocket>> vPS = MakeNullCallback<void, Ptr<MpTcpMetaSocket>> ();
  SetFullyEstablishedCallback(vPS);
}

MpTcpMetaSocket::~MpTcpMetaSocket(void)
{
  NS_LOG_FUNCTION(this);
  m_node = 0;

  if( m_scheduler )
  {

  }
  
  CancelAllEvents();

  m_subflowConnectionSucceeded = MakeNullCallback<void, Ptr<MpTcpSubflow> >();
  m_subflowConnectionFailure = MakeNullCallback<void, Ptr<MpTcpSubflow>>();
  m_subflowConnectionCreated = MakeNullCallback<void, Ptr<MpTcpSubflow>, const Address&>();
  m_joinRequest = MakeNullCallback<bool, Ptr<MpTcpMetaSocket>, const Address&, const Address&>();
  m_connectionFullyEstablished = MakeNullCallback<void, Ptr<MpTcpMetaSocket>>();
  m_subflowAdded = MakeNullCallback<void, Ptr<MpTcpSubflow>, bool> ();
}
  
TypeId
MpTcpMetaSocket::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::MpTcpMetaSocket")
  .SetParent<TcpSocketImpl>()
  .AddConstructor<MpTcpMetaSocket>()
  .AddAttribute ("SocketType",
                 "Socket type of TCP objects.",
                 TypeIdValue (MpTcpSubflow::GetTypeId ()),
                 MakeTypeIdAccessor (&MpTcpMetaSocket::m_subflowTypeId),
                 MakeTypeIdChecker ())
  .AddAttribute ("Scheduler",
                 "How to generate the mappings",
                 TypeIdValue (MpTcpSchedulerRoundRobin::GetTypeId ()),
                 MakeTypeIdAccessor (&MpTcpMetaSocket::m_schedulerTypeId),
                 MakeTypeIdChecker ())
  .AddAttribute ("TxBuffer",
                 "TCP Tx buffer",
                 PointerValue (),
                 MakePointerAccessor (&MpTcpMetaSocket::GetTxBuffer),
                 MakePointerChecker<TcpTxBuffer64> ())
  .AddAttribute ("RxBuffer",
                 "TCP Rx buffer",
                 PointerValue (),
                 MakePointerAccessor (&MpTcpMetaSocket::GetRxBuffer),
                 MakePointerChecker<TcpRxBuffer64> ())
  .AddAttribute ("TagSubflows",
                 "Append a subflow tag to the packets.",
                 BooleanValue (false),
                 MakeBooleanAccessor (&MpTcpMetaSocket::SetTagSubflows,
                                      &MpTcpMetaSocket::GetTagSubflows),
                 MakeBooleanChecker())
  // TODO rehabilitate
  //      .AddAttribute("Subflows", "The list of subflows associated to this protocol.",
  //          ObjectVectorValue(),
  //          MakeObjectVectorAccessor(&MpTcpMetaSocket::m_subflows),
  //          MakeObjectVectorChecker<MpTcpMetaSocket>())
  //        )
  
  
  ;
  return tid;
}

bool MpTcpMetaSocket::GetTagSubflows () const
{
  return m_tagSubflows;
}

void MpTcpMetaSocket::SetTagSubflows (bool value)
{
  m_tagSubflows = value;
}
  
void
MpTcpMetaSocket::CreateScheduler(TypeId schedulerTypeId)
{
  NS_LOG_FUNCTION(this);
  ObjectFactory schedulerFactory;
  schedulerFactory.SetTypeId(m_schedulerTypeId);
  m_scheduler = schedulerFactory.Create<MpTcpScheduler>();
  m_scheduler->SetMeta(this);
}

int
MpTcpMetaSocket::ConnectNewSubflow(const Address &local, const Address &remote)
{
  NS_ASSERT_MSG(InetSocketAddress::IsMatchingType(local) && InetSocketAddress::IsMatchingType(remote), "only support ipv4");
  NS_LOG_LOGIC("Trying to add a new subflow " << InetSocketAddress::ConvertFrom(local).GetIpv4()
                << "->" << InetSocketAddress::ConvertFrom(remote).GetIpv4());
  
  Ptr<MpTcpSubflow> sf = CreateSubflow(false);
  NS_ASSERT(sf);
  AddSubflow(sf, false);

  // TODO account for this error as well ?
  int ret = sf->Bind(local);
  NS_ASSERT(ret == 0);
  ret = sf->Connect(remote);

  return ret;
}
  
void
MpTcpMetaSocket::NewSubflowJoinRequest (Ptr<MpTcpSubflow> listenSubflow,
                                        Ptr<Packet> p,
                                        const TcpHeader& tcpHeader,
                                        const Address& fromAddress,
                                        const Address& toAddress,
                                        Ptr<const TcpOptionMpTcpJoin> join)
{
  NS_LOG_LOGIC("Received request for a new subflow ");
  NS_ASSERT_MSG(InetSocketAddress::IsMatchingType(fromAddress) && InetSocketAddress::IsMatchingType(toAddress),
                "Source and destination addresses should be of the same type");
  //join->GetState() == TcpOptionMpTcpJoin::Syn &&
  NS_ASSERT(join);
  NS_ASSERT(join->GetPeerToken() == GetLocalToken());
  
  // TODO check toAddress belongs to this node
  
  
  //  check we can accept the creation of a new subflow (did we receive a DSS already ?)
  if(!FullyEstablished() )
  {
    NS_LOG_WARN("Received an MP_JOIN while meta not fully established yet.");
    return;
  }
  
  //! TODO here we should trigger a callback to say if we accept the connection or not
  // (and create a helper that acts as a path manager)
  Ipv4Address ip = InetSocketAddress::ConvertFrom(toAddress).GetIpv4();
  if(!OwnIP(ip))
  {
    NS_LOG_WARN("This host does not own the ip " << ip);
    return;
  }
  
  // Similar to NotifyConnectionRequest
  bool accept_connection = NotifyJoinRequest(fromAddress, toAddress);
  if(!accept_connection)
  {
    NS_LOG_LOGIC("Refusing establishement of a new subflow");
    return;
  }
  
  //Copy the listening subflow to get all the correct tcp parameters set
  Ptr<MpTcpSubflow> subflow = CopyObject(listenSubflow);
  subflow->SetMeta(this);
  AddSubflow (subflow, false);
  
  // Call it now so that endpoint gets allocated
  subflow->CompleteFork(p, tcpHeader, fromAddress, toAddress);
}

bool
MpTcpMetaSocket::OwnIP(const Address& address) const
{
  NS_LOG_FUNCTION(this);
  NS_ASSERT_MSG(Ipv4Address::IsMatchingType(address), "only ipv4 supported for now");
  
  Ipv4Address ip = Ipv4Address::ConvertFrom(address);
  
  //  < GetNDevices()
  Ptr<Node> node = GetNode();
  Ptr<Ipv4L3Protocol> ipv4 = node->GetObject<Ipv4L3Protocol>();
  return (ipv4->GetInterfaceForAddress(ip) >= 0);
#if 0
  //  for (Ipv4InterfaceList::const_iterator it = m_interfaces.begin(); it != m_interfaces.end(); it++)
  for (uint32_t i(0); i < node->GetNDevices(); i++)
  {
    //Ptr<NetDevice> device = m_node->GetDevice(i);
    // Pourrait y avoir plusieurs IPs sur l'interface
    Ptr<NetDevice> device = node->GetDevice(i);
    // Cette fonction est mauvaise:
    if (device->GetAddress() == address)
    {
      return true;
    }
    
    //      }
  }
#endif
  return false;
}
  
void
MpTcpMetaSocket::ConnectionSucceeded (Ptr<MpTcpSubflow> subflow)
{
  if(subflow->IsMaster())
  {
    //To make sequence number mapping easier to debug, set the initial sequence number to be the master rx sequence
    m_rxBuffer->SetNextRxSequence(subflow->GetRxBuffer()->HeadSequence());
    
    m_connected = true;
    NotifyConnectionSucceeded ();
    // The if-block below was moved from ProcessSynSent() to here because we need
    // to invoke the NotifySend() only after NotifyConnectionSucceeded() to
    // reflect the behaviour in the real world.
    if (GetTxAvailable () > 0)
    {
      NotifySend (GetTxAvailable ());
    }

  }
}

uint64_t
MpTcpMetaSocket::GetLocalKey() const
{
  return m_localKey;
}

uint32_t
MpTcpMetaSocket::GetLocalToken() const
{
  return m_localToken;
}

uint32_t
MpTcpMetaSocket::GetPeerToken() const
{
  return m_peerToken;
}

uint64_t
MpTcpMetaSocket::GetPeerKey() const
{
  return m_peerKey;
}
  
enum Socket::SocketErrno MpTcpMetaSocket::GetErrno () const
{
  return m_errno;
}
  
//MpTcpAddressInfo info
// Address info
void
MpTcpMetaSocket::NotifyRemoteAddAddr(Address address)
{

  if (!m_onRemoteAddAddr.IsNull())
  {
    // TODO user should not have to deal with MpTcpAddressInfo , info.second
    m_onRemoteAddAddr (this, address, 0);
  }
}


bool
MpTcpMetaSocket::DoChecksum() const
{
  return false;
}



uint32_t
MpTcpMetaSocket::GetNActiveSubflows() const
{
  return uint32_t(m_activeSubflows.size());
}

Ptr<MpTcpSubflow>
MpTcpMetaSocket::GetActiveSubflow(uint32_t index) const
{
  NS_ASSERT_MSG(index < GetNActiveSubflows(), "Subflow index is out of range.");
  return m_subflows[index];
}

Ptr<MpTcpSubflow> MpTcpMetaSocket::GetMaster ()
{
  return m_master;
}

Ptr<MpTcpSubflow> MpTcpMetaSocket::GetSubflow(uint32_t index)
{
  NS_ASSERT_MSG(index < GetNSubflows(), "Subflow index is out of range.");
  return m_subflows[index];
}

uint32_t MpTcpMetaSocket::GetNSubflows () const
{
  return uint32_t(m_subflows.size());
}
  
void
MpTcpMetaSocket::SetPeerKey(uint64_t remoteKey)
{
  uint64_t idsn = 0;
  m_peerKey = remoteKey;
  
  GenerateTokenForKey(HMAC_SHA1, m_peerKey, m_peerToken, idsn);

  NS_LOG_DEBUG("Peer key/token set to " << m_peerKey << "/" << m_peerToken);

  //! TODO Set in TcpSocketBase an attribute to enable idsn random
  // motivation is that it's clearer to plot from 0
  if(m_tcpParams->m_nullIsn)
  {
    idsn = 0;
  }
  
//  m_rxBuffer->SetNextRxSequence(SequenceNumber32( (uint32_t)idsn ));
}
  
bool
MpTcpMetaSocket::UpdateWindowSize(uint32_t windowSize)
{
  //!
  NS_LOG_FUNCTION(this);
  m_rWnd = windowSize;
  NS_LOG_DEBUG("Meta Receiver window=" << m_rWnd);
  return true;
}

void
MpTcpMetaSocket::OnSubflowClosed(Ptr<MpTcpSubflow> subflow, bool reset)
{
  NS_LOG_LOGIC("Subflow " << subflow  << " definitely closed");
  //! TODO it should remove itself from the list of subflows and when 0 are active
  // it should call CloseAndNotify ?
  if(reset)
  {
    NS_FATAL_ERROR("Case not handled yet.");
  }

//  SubflowList::iterator it = find(m_subflows[Closing].begin(), m_subflows[Closing].end(), subflow);
// m_containers[Closing].erase(it);
//NS_ASSERT(it != m_subflows[Closing].end());
  SubflowList::iterator it = remove(m_subflows.begin(), m_subflows.end(), subflow);
}

void
MpTcpMetaSocket::DumpRxBuffers(Ptr<MpTcpSubflow> sf) const
{
  NS_LOG_INFO("=> Dumping meta RxBuffer ");
  m_rxBuffer->Dump();

  // TODO parcourir les sous flots

  for(int i = 0; i < (int)GetNActiveSubflows(); ++i)
  {
    Ptr<MpTcpSubflow> sf = m_activeSubflows[i];
    NS_LOG_INFO("=> Rx Buffer of subflow=" << sf);
    sf->m_rxBuffer->Dump();
  }
}
  
bool MpTcpMetaSocket::AddToReceiveBuffer(Ptr<MpTcpSubflow> sf,
                                         Ptr<Packet> p,
                                         const TcpHeader& tcpHeader,
                                         Ptr<MpTcpMapping> mapping)
{
  //Add the packet to the receive buffer.
  //We shouldn't use HeadDSN, but rather the actual dsn number based on the SSN.
  SequenceNumber64 dsn = mapping->GetDSNFromSSN(tcpHeader.GetSequenceNumber());
  if (!m_rxBuffer->Add(p, dsn))
  { // Insert failed: No data or RX buffer full
    NS_LOG_WARN("Insert failed, No data (" << p->GetSize() << ") ?");
    m_rxBuffer->Dump();
    
    return false;
  }
  return true;
}
  
void MpTcpMetaSocket::OnSubflowRecv(Ptr<MpTcpSubflow> sf,
                                    Ptr<Packet> p,
                                    const TcpHeader& tcpHeader,
                                    SequenceNumber64 expectedDSN,
                                    Ptr<MpTcpMapping> mapping)
{
  NS_LOG_FUNCTION(this << "Received data from subflow=" << sf);
  
  NS_LOG_INFO("=> Dumping meta RxBuffer before extraction");
  DumpRxBuffers(sf);
  
  // Remove mappings for contiguous sequence numbers, and notify the application
  if (expectedDSN < m_rxBuffer->NextRxSequence())
  {
    NS_LOG_LOGIC("The Rxbuffer advanced");
    
    // NextRxSeq advanced, we have something to send to the app
    if (!m_tcpParams->m_shutdownRecv)
    {
      NS_LOG_LOGIC("Notify data Rcvd" );
      NotifyDataRecv();
      
      // Handle exceptions
      if (m_tcpParams->m_closeNotified)
      {
        NS_LOG_WARN ("The socket " << this << " received data after close notification!");
      }
      // If we received FIN before and now completed all "holes" in rx buffer,
      // invoke peer close procedure
      if (m_rxBuffer->Finished ())
      {
        DoPeerClose (sf);
      }
    }
  }
}

/**
TODO check that it autodisconnects when we destroy the object ?
**/
void
MpTcpMetaSocket::OnSubflowUpdateCwnd(Ptr<MpTcpSubflow> subflow, uint32_t oldCwnd, uint32_t newCwnd)
{
  NS_LOG_LOGIC("Subflow updated window from " << oldCwnd << " to " << newCwnd
//        << " (context=" << context << ")"
        );

  //TODO: this was used to compute total cwnd, that is not really valid for a meta socket.
  
  if ((newCwnd > oldCwnd) && (!m_sendPendingDataEvent.IsRunning ()))
  {
    m_sendPendingDataEvent = Simulator::Schedule (TimeStep (1),
                                                  &MpTcpMetaSocket::SendPendingData,
                                                  this);
  }
}

  MpTcpMetaSocket::SubflowList MpTcpMetaSocket::GetSubflowsWithState(TcpStates_t state)
{
  SubflowList subflows;
  for (SubflowList::iterator it = m_subflows.begin(); it != m_subflows.end(); ++it)
  {
    Ptr<MpTcpSubflow> sf = (*it);
    if(sf->GetState() == state)
    {
      subflows.push_back(sf);
    }
  }
  return subflows;
}

  

/**
TODO add a MakeBoundCallback that accepts a member function as first input
**/
void MpTcpMetaSocket::NotifySubflowNewState(Ptr<MpTcpMetaSocket> meta,
                                            Ptr<MpTcpSubflow> sf,
                                            TcpSocket::TcpStates_t  oldState,
                                            TcpSocket::TcpStates_t newState)
{
    meta->OnSubflowNewState(sf, oldState, newState);
}
  
void MpTcpMetaSocket::NotifySubflowUpdateCwnd(Ptr<MpTcpMetaSocket> meta,
                                              Ptr<MpTcpSubflow> sf,
                                              uint32_t oldCwnd,
                                              uint32_t newCwnd)
{
  meta->OnSubflowUpdateCwnd(sf, oldCwnd, newCwnd);
}

/**
TODO use it to Notify user of
We need a MakeBoundCallback
*/
void
MpTcpMetaSocket::OnSubflowNewState(Ptr<MpTcpSubflow> sf,
                                   TcpSocket::TcpStates_t  oldState,
                                   TcpSocket::TcpStates_t newState)
{
  NS_LOG_LOGIC("subflow " << sf << " state changed from " << TcpStateName[oldState] << " to " << TcpStateName[newState]);
  NS_LOG_LOGIC("Current rWnd=" << m_rWnd);
  
  if(sf->IsMaster() && newState == SYN_RCVD)
  {
      //!
      NS_LOG_LOGIC("moving meta to SYN_RCVD");
      m_rWnd = sf->m_rWnd;
  }
  
  switch(newState)
  {
    case LISTEN:
    {
      if (sf->IsMaster())
      {
        m_state = MptcpMetaListening;
      }
      break;
    }
    case SYN_SENT:
    case SYN_RCVD:
    {
      if (sf->IsMaster())
      {
        m_state = MptcpMetaPreEstablished;
      }
      break;
    }
    case ESTABLISHED:
    {
      if (sf->IsMaster())
      {
        m_state = MptcpMetaEstablished;
      }
      
      //Add the subflow to the active subflows list
      m_activeSubflows.push_back(sf);
      
      // subflow did SYN_RCVD -> ESTABLISHED
      if(oldState == SYN_RCVD)
      {
        NS_LOG_LOGIC("Subflow created");
        OnSubflowEstablishment(sf);
      }
      // subflow did SYN_SENT -> ESTABLISHED
      else if(oldState == SYN_SENT)
      {
        //OnSubflowEstablishment(sf);
      }
      else
      {
        NS_FATAL_ERROR("Unhandled case");
      }
      break;
    }
    default:
    {
      //nothing to do
    }
  }
}

Ptr<MpTcpSubflow>
MpTcpMetaSocket::CreateSubflow(bool masterSocket)
{
  NS_LOG_FUNCTION (this << m_subflowTypeId.GetName());
  
  Ptr<Socket> socket = m_tcp->CreateSocket(m_congestionControl, m_subflowTypeId);
  Ptr<MpTcpSubflow> subflow = DynamicCast<MpTcpSubflow>(socket);
  
  //Set the subflow parameters
  subflow->SetTcpParameters(m_tcpParams);
  subflow->SetInitialCwnd(m_initialCWnd);
  subflow->SetInitialSSThresh(m_initialSsThresh);
  subflow->SetSegSize(m_segmentSize);
  subflow->SetSndBufSize(GetSndBufSize());
  subflow->SetRcvBufSize(GetRcvBufSize());
  
  subflow->SetMeta(this);
  
  subflow->m_masterSocket = masterSocket;
  
  // The close on empty flag should not be true for the subflows. The connection
  // (i.e. meta socket) needs to send DATA_FIN before we close the subflows
  // Also, the check to see if there is any remaining data should
  // be performed on the connection level tx buffer.
  subflow->m_tcpParams->m_closeOnEmpty = false;

  return subflow;
}

void MpTcpMetaSocket::CreateMasterSubflow ()
{
  NS_ASSERT(!m_master);
  AddSubflow(CreateSubflow(true), true);
  
  //If we haven't generated the key yet, do it here
  if (m_localKey == 0)
  {
    GenerateUniqueMpTcpKey();
  }
}
  
void
MpTcpMetaSocket::AddSubflow(Ptr<MpTcpSubflow> sflow, bool isMaster)
{
  NS_LOG_FUNCTION(sflow);
  
  bool ok = true;
  
  //Let the meta socket know when the subflow TCP state has changed.
  sflow->TraceConnectWithoutContext ("State", MakeBoundCallback(&MpTcpMetaSocket::NotifySubflowNewState, this, sflow));
  NS_ASSERT(ok);
  
  //The meta socket should be notified when a subflow's congestion window changes so it can attempt to send pending data
  sflow->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback(&MpTcpMetaSocket::NotifySubflowUpdateCwnd, this, sflow));
  NS_ASSERT(ok);
  
  sflow->SetDataSentCallback (MakeCallback(&MpTcpMetaSocket::NotifySubflowDataSent, this));
  
  sflow->SetAcceptCallback (MakeCallback(&MpTcpMetaSocket::NotifySubflowConnectionRequest, this),
                            MakeCallback(&MpTcpMetaSocket::NotifySubflowNewConnectionCreated, this));
  
  sflow->SetConnectCallback (MakeCallback (&MpTcpMetaSocket::NotifySubflowConnectionSuccess,this),
                             MakeCallback (&MpTcpMetaSocket::NotifySubflowConnectionFailure,this));
  
  if(isMaster)
  {
    m_master = sflow;
    sflow->SetMaster();
  }
  
  sflow->SetSubflowId(uint32_t(m_subflows.size()));
  m_subflows.push_back(sflow);
  
  if(sflow->GetState() == ESTABLISHED)
  {
    m_activeSubflows.push_back(sflow);
  }
  
  if(!m_subflowAdded.IsNull())
  {
    m_subflowAdded(sflow, isMaster);
  }
}

bool
MpTcpMetaSocket::NotifyJoinRequest (const Address &from, const Address & toAddress)
{
  NS_LOG_FUNCTION (this << &from);
  if (!m_joinRequest.IsNull ())
  {
    return m_joinRequest (this, from, toAddress);
  }
  else
  {
    // accept all incoming connections by default.
    // this way people writing code don't have to do anything
    // special like register a callback that returns true
    // just to get incoming connections
    return true;
  }
}

bool MpTcpMetaSocket::NotifySubflowConnectionRequest(Ptr<Socket> socket, const Address &from)
{
  Ptr<MpTcpSubflow> subflow = DynamicCast<MpTcpSubflow>(socket);
  if(subflow->IsMaster())
  {
    return NotifyConnectionRequest(from);
  }
  else
  {
    //This is a join request, dealt with in MpTcpSubflow ProcessListen seperately, which
    //calls the NotifyJoinRequest callback. Should never be called.
    NS_FATAL_ERROR("Shouldn't be called");
    return true;
  }
}

void MpTcpMetaSocket::NotifySubflowNewConnectionCreated(Ptr<Socket> socket, const Address &from)
{
  Ptr<MpTcpSubflow> subflow = DynamicCast<MpTcpSubflow>(socket);
  if(subflow->IsMaster())
  {
    NotifyNewConnectionCreated(this, from);
  }
  else if (!m_subflowConnectionCreated.IsNull ())
  {
    m_subflowConnectionCreated(subflow, from);
  }
}

void
MpTcpMetaSocket::NotifySubflowConnectionSuccess (Ptr<Socket> socket)
{
  NS_LOG_LOGIC(this);
  Ptr<MpTcpSubflow> subflow = DynamicCast<MpTcpSubflow>(socket);
  if(subflow->IsMaster())
  {
    //The master subflow's success indicates that the meta socket has succeeded in connecting
    //But the NotifyConnectionSuccess is dealt with in the ConnectionSucceeded call.
  }
  else if (!m_subflowConnectionSucceeded.IsNull ())
  {
    m_subflowConnectionSucceeded (subflow);
  }
}

void
MpTcpMetaSocket::NotifySubflowConnectionFailure (Ptr<Socket> socket)
{
    NS_LOG_LOGIC(this);
    Ptr<MpTcpSubflow> sf = DynamicCast<MpTcpSubflow>(socket);
    if(sf->IsMaster())
    {
        NotifyConnectionFailed();
    }
    else
    {
        // use a specific callback
        NS_FATAL_ERROR("TODO");
    }
}
  
void MpTcpMetaSocket::NotifySubflowDataSent(Ptr<Socket> socket, uint32_t dataSent)
{
  NotifyDataSent(dataSent);
}

void MpTcpMetaSocket::NotifyFullyEstablished ()
{
  if (!m_connectionFullyEstablished.IsNull())
  {
    m_connectionFullyEstablished(this);
  }
}
  
void
MpTcpMetaSocket::OnInfiniteMapping(Ptr<TcpOptionMpTcpDSS> dss, Ptr<MpTcpSubflow> sf)
{
  NS_FATAL_ERROR("Infinite mapping not implemented");
}


/*
This should take care of ReTxTimeout
*/
void
MpTcpMetaSocket::NewAck(Ptr<MpTcpSubflow> subflow, const SequenceNumber64& dsn)
{
  NS_LOG_FUNCTION(this << " new dataack=[" <<  dsn << "]");
  
  //
  if ((subflow->IsMaster()) && (subflow->GetState() == SYN_SENT) && (m_state = MptcpMetaPreEstablished))
  {
    return;
  }
  /*From the RFC6824:
   An MPTCP sender MUST NOT free data from the send buffer until it has
   been acknowledged by both a Data ACK received on any subflow and at
   the subflow level by all subflows on which the data was sent.
   
   The above is only true if the subflows don't keep a copy of the full packet in their tx buffers,
   but rather a pointer to the packet in the connection level tx buffer. Therefore, for our purposes
   it is safe to delete the packet from the connection, and let subflows deal with managing their own
   tx buffers.
   */
  
  /*
   DiscardUpTo  Discard data up to but not including this sequence number.
   */
  m_txBuffer->DiscardUpTo(dsn);
  
  if (dsn > m_nextTxSequence)
  {
    m_nextTxSequence = dsn; // If advanced
  }
  
  if (GetTxAvailable() > 0)
  {
    NS_LOG_INFO("Tx available" << GetTxAvailable());
    NotifySend(GetTxAvailable());
  }
  
  // Try to send more data
  /*if (!m_sendPendingDataEvent.IsRunning ())
  {
    m_sendPendingDataEvent = Simulator::Schedule (TimeStep (1),
                                                  &MpTcpMetaSocket::SendPendingData,
                                                  this, m_connected);
  }*/
}

bool
MpTcpMetaSocket::IsInfiniteMappingEnabled() const
{
    return false;
}

// Send 1-byte data to probe for the window size at the receiver when
// the local knowledge tells that the receiver has zero window size
// C.f.: RFC793 p.42, RFC1112 sec.4.2.2.17
void
MpTcpMetaSocket::PersistTimeout()
{
  NS_LOG_LOGIC ("PersistTimeout expired at " << Simulator::Now ().GetSeconds ());
  NS_FATAL_ERROR("TODO");
}

void MpTcpMetaSocket::SetFullyEstablishedCallback (Callback<void, Ptr<MpTcpMetaSocket>> callback)
{
  m_connectionFullyEstablished = callback;
}

void
MpTcpMetaSocket::BecomeFullyEstablished()
{
  NS_LOG_FUNCTION (this);
  m_receivedDSS = true;

  NotifyFullyEstablished();
}

bool
MpTcpMetaSocket::FullyEstablished() const
{
    NS_LOG_FUNCTION_NOARGS();
    return m_receivedDSS;
}
  
SequenceNumber64 MpTcpMetaSocket::GetNextTxSequence() const
{
  return m_nextTxSequence;
}

/**
 * Sending data via subflows with available window size.
 * we should not care about IsInfiniteMapping()
 */
bool
MpTcpMetaSocket::SendPendingData()
{
  NS_LOG_FUNCTION(this << "Sending data");

  
  if (m_txBuffer->Size () == 0)
    {
      NS_LOG_DEBUG("Nothing to send");
      return false;                           // Nothing to send
    }
  
  int nbMappingsDispatched = 0; // mimic nbPackets in TcpSocketBase::SendPendingData
  
  while (m_txBuffer->SizeFromSequence(m_nextTxSequence))
  {
    uint32_t dataToSend = m_txBuffer->SizeFromSequence(m_nextTxSequence);
    uint32_t metaWindow = AvailableWindow();
    
    if(metaWindow <= 0)
    {
      NS_LOG_DEBUG("No meta window available (TODO should be in persist state ?)");
      break;
    }
    
    //Ask the scheduler for a possible subflow to send on
    Ptr<MpTcpSubflow> subflow = m_scheduler->GetAvailableSubflow(dataToSend, metaWindow);
    
    if(!subflow)
    {
      break;
    }
    
    uint32_t length = m_scheduler->GetSendSizeForSubflow(subflow, subflow->GetSegSize(), dataToSend);
    
    //Create the DSN->SSN mapping in the subflow
    Ptr<MpTcpMapping> mapping = subflow->AddLooseMapping(m_nextTxSequence, length);
    NS_ASSERT(mapping);
    
    //Send packet on subflow right away
    Ptr<Packet> p = m_txBuffer->CopyFromSequence(length, m_nextTxSequence);
    
    int ret = subflow->Send(p, 0);
    
    NS_LOG_DEBUG("Send result=" << ret);
    
    /*
     Ideally we should be able to send data out of order so that it arrives in order at the
     receiver but to do that we need SACK support (IMO). Once SACK is implemented it should
     be reasonably easy to add
     */
    
    m_nextTxSequence += length;
    m_highTxMark += length;
    nbMappingsDispatched++;
    
    NS_LOG_LOGIC("m_nextTxSequence=" << m_nextTxSequence);
    
  }

//  NS_LOG_LOGIC ("Dispatched " << nPacketsSent << " mappings");
  return nbMappingsDispatched > 0;
}

/**
 TCP: Upon RTO:
 1) GetSSThresh() is set to half of flight size
 2) cwnd is set to 1*MSS
 3) retransmit the lost packet
 4) Tcp back to slow start
 */
//
//void
//MpTcpMetaSocket::ReTxTimeout(uint8_t sFlowIdx)
//{ // Retransmit timeout
//  NS_LOG_FUNCTION (this);
////  NS_ASSERT_MSG(client, "ReTxTimeout is not implemented for server side yet");
//  Ptr<MpTcpSubflow> sFlow = m_subflows[sFlowIdx];
//
//  NS_LOG_INFO ("Subflow ("<<(int)sFlowIdx<<") ReTxTimeout Expired at time "
//        << Simulator::Now ().GetSeconds()<< " unacked packets count is "<<sFlow->m_mapDSN.size()
//        << " sFlow->state: " << TcpStateName[sFlow->m_state]
//        ); //
//  //NS_LOG_INFO("TxSeqNb: " << sFlow->TxSeqNumber << " HighestAck: " << sFlow->highestAck);
//  // If erroneous timeout in closed/timed-wait state, just return
//  if (sFlow->m_state == CLOSED || sFlow->m_state  == TIME_WAIT)
//    {
//      NS_LOG_INFO("RETURN");
//      NS_ASSERT(3!=3);
//      return;
//    }
//  // If all data are received (non-closing socket and nothing to send), just return
//  // if (m_state <= ESTABLISHED && m_txBuffer->HeadSequence() >= m_highTxMark)
//  if (sFlow->m_state  <= ESTABLISHED && sFlow->m_mapDSN.size() == 0)
//    {
//      NS_LOG_INFO("RETURN");
//      NS_ASSERT(3!=3);
//      return;
//    }
//  Retransmit(sFlowIdx); // Retransmit the packet
//}

void
MpTcpMetaSocket::OnSubflowDupAck(Ptr<MpTcpSubflow> sf)
{
  NS_LOG_DEBUG("Dup ack signaled by subflow " << sf );

}

/**
Retransmit timeout

This function should be very interesting because one may
adopt different strategies here, like reinjecting on other subflows etc...
Maybe allow for a callback to be set here.
*/
void
MpTcpMetaSocket::Retransmit()
{
  NS_LOG_LOGIC(this);
  NS_FATAL_ERROR("TODO: fix implementation");
#if 0
//  NS_FATAL_ERROR("TODO reestablish retransmit ?");
//  NS_LOG_ERROR("TODO");
  m_nextTxSequence = FirstUnackedSeq(); // Start from highest Ack
//  m_rtt->IncreaseMultiplier(); // Double the timeout value for next retx timer
  m_dupAckCount = 0;
  DoRetransmit(); // Retransmit the packet
//  TcpSocketBase::Retransmit();
#endif
}



void
MpTcpMetaSocket::DoRetransmit()
{
  NS_LOG_FUNCTION (this);

  NS_FATAL_ERROR("TODO: fix implementation");
  
#if 0

  // Retransmit SYN packet
  if (m_state == SYN_SENT)
    {
      if (m_synCount > 0)
        {
          // TODO
          NS_FATAL_ERROR("TODO, first syn didn't reach it should be resent. Maybe this should be let to the subflow");
//          SendEmptyPacket(TcpHeader::SYN);
        }
      else
        {
          NotifyConnectionFailed();
        }
      return;
    }

  // Retransmit non-data packet: Only if in FIN_WAIT_1 or CLOSING state
  if (m_txBuffer->Size() == 0)
    {
      if (m_state == FIN_WAIT_1 || m_state == CLOSING)
        {
          // Must have lost FIN, re-send
//          SendEmptyPacket(TcpHeader::FIN);
          TcpHeader header;
          SendFin();

        }
      return;
    }
  // Retransmit a data packet: Call SendDataPacket
  NS_LOG_LOGIC ("TcpSocketBase " << this << " retxing seq " << FirstUnackedSeq ());

//  m_rxBuffer->Dump();

  DumpRxBuffers(0);

//  SendDataPacket();
  NS_FATAL_ERROR("TODO later, but for the tests only, it should not be necesssary ?! Check for anything suspicious");
//
//  m_nextTxSequence = FirstUnackedSeq();
//  SendPendingData(true);
//

  // normally here m_nextTxSequence has been set to firstUna
//  uint32_t sz = SendDataPacket(, m_segmentSize, true);
//  // In case of RTO, advance m_nextTxSequence
//  m_nextTxSequence = max(m_nextTxSequence.Get(), FirstUnackedSeq() + sz);
  //reTxTrack.push_back(make_pair(Simulator::Now().GetSeconds(), ns3::TcpNewReno::cWnd));
#endif
}

void
MpTcpMetaSocket::SendDataFin(bool withAck)
{
  //Get a subflow from the scheduler
  Ptr<MpTcpSubflow> subflow = m_scheduler->GetAvailableControlSubflow();
  
  NS_ASSERT(subflow);
  
  TcpHeader header;
  subflow->GenerateEmptyPacketHeader(header,TcpHeader::ACK);
  subflow->AppendDSSFin();
  if(withAck)
  {
    subflow->AppendDSSAck();
  }
  subflow->SendEmptyPacket(header);
  
  //Schedule a retransmit of the DATA FIN if it is not Data Acked, to guard against loss
  if (m_retxEvent.IsExpired () && !withAck )
  {
    //Use the RTO of the subflow we sent the DATA FIN on
    NS_LOG_LOGIC ("Schedule retransmission timeout at time "
                  << Simulator::Now ().GetSeconds () << " to expire at time "
                  << (Simulator::Now () + subflow->m_rto.Get ()).GetSeconds ());
    m_retxEvent = Simulator::Schedule (subflow->m_rto, &MpTcpMetaSocket::SendDataFin, this, withAck);
  }
}

void
MpTcpMetaSocket::SendDataAck (Ptr<MpTcpSubflow> sf)
{
  //Send a Data Ack
  TcpHeader header;
  sf->GenerateEmptyPacketHeader(header, TcpHeader::ACK);
  sf->AppendDSSAck();
  sf->SendEmptyPacket(header);
}

void
MpTcpMetaSocket::ReTxTimeout()
{
  NS_LOG_FUNCTION(this);
  NS_FATAL_ERROR("TODO: fix implementation");
  
  //return TcpSocketBase::ReTxTimeout();
}
  
uint64_t
MpTcpMetaSocket::GenerateUniqueMpTcpKey()
{
  NS_LOG_FUNCTION("Generating key");
  NS_ASSERT(m_tcp);
  
  uint64_t idsn;
  
  Ptr<UniformRandomVariable> uniformVar = CreateObject<UniformRandomVariable>() ;
  uniformVar->SetAttribute("Min", DoubleValue(1));
  uniformVar->SetAttribute("Max", DoubleValue(numeric_limits<uint64_t>::max()));
  
  do
  {
    m_localKey = uniformVar->GetValue();
    GenerateTokenForKey(HMAC_SHA1, m_localKey, m_localToken, idsn);
  }
  while(m_tcp->LookupMpTcpToken(m_localToken));
  
  //Tell the TcpL4Protocol the token->socket mapping
  m_tcp->AddTokenMapping(m_localToken, this);
  
  
  //  NS_LOG_DEBUG("Key/token set to " << localKey << "/" << localToken);
  //  NS_LOG_DEBUG("Key/token set to " << m_mptcpLocalKey << "/" << m_mptcpLocalToken);
  //  uint64_t idsn = 0;
  //  /**
  //
  //  /!\ seq nb must be 64 bits for mptcp but that would mean rewriting lots of code so
  //
  //  TODO add a SetInitialSeqNb member into TcpSocketBase
  //  **/
  //  if(m_nullIsn)
  //  {
  //    m_nextTxSequence = (uint32_t)0;
  //  }
  //  else
  //  {
  //    m_nextTxSequence = (uint32_t)idsn;
  //  }
  //
  ////  SetTxHead(m_nextTxSequence);
  //  m_firstTxUnack = m_nextTxSequence;
  //  m_highTxMark = m_nextTxSequence;
  
  return m_localKey;
  
}
  


//void
//MpTcpMetaSocket::GetIdManager()
//{
//  NS_ASSERT(m_remotePathIdManager);
//  return m_remotePathIdManager;
//}

void
MpTcpMetaSocket::GetAllAdvertisedDestinations(vector<InetSocketAddress>& cont)
{
  NS_ASSERT(m_remotePathIdManager);
  m_remotePathIdManager->GetAllAdvertisedDestinations(cont);
}


void
MpTcpMetaSocket::SetNewAddrCallback(Callback<bool, Ptr<Socket>, Address, uint8_t> remoteAddAddrCb,
                          Callback<void, uint8_t> remoteRemAddrCb)

{
  //
  m_onRemoteAddAddr = remoteAddAddrCb;
  m_onAddrDeletion = remoteRemAddrCb;
}

void
MpTcpMetaSocket::DumpSubflows() const
{
  NS_LOG_FUNCTION(this << "\n");
  
  for( SubflowList::const_iterator it = m_subflows.begin(); it != m_subflows.end(); it++ )
  {
    NS_LOG_UNCOND("- subflow [" << *it  << "]");
  }
}


/*
TODO rename into subflow created
*/
void
MpTcpMetaSocket::OnSubflowEstablished(Ptr<MpTcpSubflow> subflow)
{

    // from
    InetSocketAddress addr(subflow->m_endPoint->GetPeerAddress(), subflow->m_endPoint->GetPeerPort());
//    NotifyNewConnectionCreated(subflow, addr);
    NotifyNewConnectionCreated(this, addr);
  
//  else
//  {
//  Simulator::ScheduleNow(&MpTcpMetaSocket::NotifySubflowCreated, this, subflow );
//  }
}

// TODO this could be done when tracking the subflow m_state
void
MpTcpMetaSocket::OnSubflowEstablishment(Ptr<MpTcpSubflow> subflow)
{

  NS_LOG_LOGIC(this << " (=meta) New subflow " << subflow << " established");
  
  if (subflow->IsMaster())
  {
    m_highTxMark = ++m_nextTxSequence;
    m_txBuffer->SetHeadSequence (m_nextTxSequence);
  }

//  Simulator::ScheduleNow(&MpTcpMetaSocket::NotifySubflowConnected, this, subflow);
}
  
void MpTcpMetaSocket::EstablishSubflow(Ptr<MpTcpSubflow> subflow,
                                       Ptr<Packet> packet,
                                       const TcpHeader& tcpHeader)
{
  if (subflow->IsMaster())
  {
    m_highTxMark = ++m_nextTxSequence;
    m_txBuffer->SetHeadSequence (m_nextTxSequence);
    //TODO: the first rx sequence should be based off of the MPTCP key
    m_rxBuffer->SetNextRxSequence (SequenceNumber32 (1));
    
  }
  SendPendingData();
}

//
void
MpTcpMetaSocket::SetSubflowAcceptCallback(Callback<bool, Ptr<MpTcpMetaSocket>, const Address&, const Address&> joinRequest,
                                          Callback<void, Ptr<MpTcpSubflow>, const Address&> connectionCreated)
{
  NS_LOG_FUNCTION(this << &joinRequest << " " << &connectionCreated);
  m_joinRequest = joinRequest;
  m_subflowConnectionCreated = connectionCreated;
}

void
MpTcpMetaSocket::SetSubflowConnectCallback(Callback<void, Ptr<MpTcpSubflow> > connectionSucceeded,
                                           Callback<void, Ptr<MpTcpSubflow> > connectionFailure)
{
  NS_LOG_FUNCTION(this << &connectionSucceeded);
  m_subflowConnectionSucceeded = connectionSucceeded;
  m_subflowConnectionFailure = connectionFailure;
}
  
void
MpTcpMetaSocket::SetSubflowAddedCallback(Callback<void, Ptr<MpTcpSubflow>, bool> subflowAdded)
{
  NS_LOG_FUNCTION(this << &subflowAdded);
  m_subflowAdded = subflowAdded;
}

TypeId
MpTcpMetaSocket::GetInstanceTypeId(void) const
{
  return MpTcpMetaSocket::GetTypeId();
}



void
MpTcpMetaSocket::OnSubflowClosing(Ptr<MpTcpSubflow> sf)
{
  NS_LOG_LOGIC("Subflow has gone into state ["
//               << TcpStateName[sf->m_state]
               );

//
//  /* if this is the last active subflow
//
//  */
//  //FIN_WAIT_1
//  switch( sf->m_state)
//  {
//    case FIN_WAIT_1:
//    case CLOSE_WAIT:
//    case LAST_ACK:
//    default:
//      break;
//  };


  
  //      #TODO I need to Ack the DataFin in (DoPeerCLose)
}


void
MpTcpMetaSocket::OnSubflowDupack(Ptr<MpTcpSubflow> sf, MpTcpMapping mapping)
{
  NS_LOG_LOGIC("Subflow Dupack TODO.Nothing done by meta");
}

void
MpTcpMetaSocket::OnSubflowRetransmit(Ptr<MpTcpSubflow> sf)
{
  NS_LOG_INFO("Subflow retransmit. Nothing done by meta");
}

uint32_t MpTcpMetaSocket::UnAckDataCount()
{
  NS_LOG_FUNCTION (this);
  return uint32_t(m_nextTxSequence.Get() - m_txBuffer->HeadSequence());
}

uint32_t MpTcpMetaSocket::GetRwndSize ()
{
  return m_rWnd;
}
  
//this would not accomodate with google option that proposes to add payload in
// syn packets MPTCP
/**
 cf RFC:
 
 To compute cwnd_total, it is an easy mistake to sum up cwnd_i across
 all subflows: when a flow is in fast retransmit, its cwnd is
 typically inflated and no longer represents the real congestion
 window.  The correct behavior is to use the ssthresh (slow start
 threshold) value for flows in fast retransmit when computing
 cwnd_total.  To cater to connections that are app limited, the
 computation should consider the minimum between flight_size_i and
 cwnd_i, and flight_size_i and ssthresh_i, where appropriate.
 
 TODO fix this to handle flight size
 **/

uint32_t MpTcpMetaSocket::GetTotalCwnd ()
{
  uint32_t totalCwnd = 0;
  for (SubflowList::iterator it = m_activeSubflows.begin(); it != m_activeSubflows.end(); ++it)
  {
    Ptr<MpTcpSubflow> subflow = (*it);
    // when in fast recovery, use SS threshold instead of cwnd
    if(subflow->m_tcb->m_congState == TcpSocketState::CA_RECOVERY)
    {
      NS_LOG_DEBUG("Is in Fast recovery");
      totalCwnd += subflow->m_tcb->m_ssThresh;
    }
    else
    {
      totalCwnd += subflow->m_tcb->m_cWnd;
    }
  }
  return totalCwnd;
}

uint32_t
MpTcpMetaSocket::AvailableWindow()
{
  NS_LOG_FUNCTION(this);
  uint32_t rwnd = GetRwndSize ();
  uint32_t unack = UnAckDataCount ();
  
  NS_LOG_DEBUG ("UnAckCount=" << unack << ", Win=" << rwnd);
  return (rwnd < unack) ? 0 : (rwnd - unack);
}

void
MpTcpMetaSocket::SendFastClose(Ptr<MpTcpSubflow> sf)
{
  NS_LOG_LOGIC ("Sending MP_FASTCLOSE");
  NS_FATAL_ERROR("TODO");
  
#if 0
  // TODO: send an MPTCP_fail
  TcpHeader header;
  //   Ptr<MpTcpSubflow> sf = GetSubflow(0);
  sf->GenerateEmptyPacketHeader(header, TcpHeader::RST);
  Ptr<TcpOptionMpTcpFastClose> opt = Create<TcpOptionMpTcpFastClose>();
  
  opt->SetPeerKey(GetPeerKey());
  
  sf->SendEmptyPacket(header);
  
  //! Enter TimeWait ?
  //  NotifyErrorClose();
  TimeWait();
  //  DeallocateEndPoint();
#endif
}


void
MpTcpMetaSocket::ReceivedAck(Ptr<MpTcpSubflow> sf, const SequenceNumber64& dack)
{
//
  NS_LOG_FUNCTION("Received DACK " << dack << "from subflow" << sf << "(Enable dupacks:"  << " )");

  TcpStates_t subflowState = sf->GetState();
  
  if((subflowState == SYN_SENT) && (m_state == MptcpMetaPreEstablished) && sf->IsMaster())
  {
    return;
  }
  
  SequenceNumber64 firstUnacked = m_txBuffer->HeadSequence();
  if (dack < firstUnacked)
  { // Case 1: Old ACK, ignored.
    NS_LOG_LOGIC ("Old ack Ignored " << dack);
  }
  else if (dack  == firstUnacked)
  { // Case 2: Potentially a duplicated ACK
    NS_LOG_LOGIC ("Duplicate ack ignored " << dack);
  }
  else if (dack  > firstUnacked)
  { // Case 3: New ACK, reset m_dupAckCount and update m_txBuffer
    NS_LOG_LOGIC ("New DataAck [" << dack  << "]");
    
    NewAck(sf, dack);
  }
  
  if (m_state == MptcpMetaFinWait1 && m_txBuffer->Size () == 0
      && (dack == m_highTxMark + SequenceNumber32 (1)))
  {
    // This ACK corresponds to the FIN sent
    NS_LOG_DEBUG ("FIN_WAIT_1 -> FIN_WAIT_2");
    m_state = MptcpMetaFinWait2;
    
    //We've received an in order DATA ACK for the DATA FIN sent, we need to close
    //all the subflows
    CloseAllSubflows();
  }
  else if (m_state == MptcpMetaClosing)
  {
    CloseAllSubflows();
    // This ACK corresponds to the FIN sent
    TimeWait ();
  }
  else if (m_state == MptcpMetaLastAck)
  {
    CloseAllSubflows();
    CloseAndNotify();
  }
  else
  {
  }
}

/** Move TCP to Time_Wait state and schedule a transition to Closed state */
void
MpTcpMetaSocket::TimeWait()
{
  NS_LOG_FUNCTION(this);
  
  Time duration = Seconds(2 * m_tcpParams->m_msl);
  m_state = MptcpMetaTimeWait;
  CancelAllEvents();
//  // Move from TIME_WAIT to CLOSED after 2*MSL. Max segment lifetime is 2 min
//  // according to RFC793, p.28
  m_timeWaitEvent = Simulator::Schedule(duration, &MpTcpMetaSocket::CloseAndNotify, this);
}

void
MpTcpMetaSocket::OnTimeWaitTimeOut(void)
{
  // Would normally call CloseAndNotify
  NS_LOG_LOGIC("Timewait timeout expired");
  NS_LOG_UNCOND("after timewait timeout, there are still " << m_subflows.size() << " subflows not closed");

  CloseAndNotify();
}

/** Peacefully close the socket by notifying the upper layer and deallocate end point */
void
MpTcpMetaSocket::CloseAndNotify(void)
{
  NS_LOG_FUNCTION (this);

  // TODO check the number of open subflows
  if (!m_tcpParams->m_closeNotified)
    {
      NotifyNormalClose();
    }
 
  m_tcpParams->m_closeNotified = true;
  
  CancelAllEvents();
  m_state = MptcpMetaClosed;
}

void
MpTcpMetaSocket::LastAckTimeout (void)
{
  NS_LOG_FUNCTION (this);
  
  m_lastAckEvent.Cancel ();
  if (m_state == MptcpMetaLastAck)
  {
    CloseAndNotify ();
  }
  if (!m_tcpParams->m_closeNotified)
  {
    m_tcpParams->m_closeNotified = true;
  }
}
  
/*
 Notably, it is only DATA_ACKed once all
 data has been successfully received at the connection level.  Note,
 therefore, that a DATA_FIN is decoupled from a subflow FIN.  It is
 only permissible to combine these signals on one subflow if there is
 no data outstanding on other subflows.
 */

//We've received a DATA_FIN from a peer.
void
MpTcpMetaSocket::PeerClose(const SequenceNumber64& dsn, const SequenceNumber64& dack, Ptr<MpTcpSubflow> sf)
{
  NS_LOG_LOGIC("Datafin with seq=" << dsn);
  
  if(dsn < m_rxBuffer->NextRxSequence() || m_rxBuffer->MaxRxSequence() < dsn)
  {
    NS_LOG_INFO("dsn " << dsn << " out of expected range [ " << m_rxBuffer->NextRxSequence()  << " - " << m_rxBuffer->MaxRxSequence() << " ]" );
    return ;
  }
  
  // For any case, remember the FIN position in rx buffer first
  //! +1 because the datafin doesn't count as payload
  m_rxBuffer->SetFinSequence(dsn);
  NS_LOG_LOGIC ("Accepted MPTCP FIN at seq " << dsn);
  
  // Return if FIN is out of sequence, otherwise move to CLOSE_WAIT state by DoPeerClose
  if (!m_rxBuffer->Finished())
  {
    NS_LOG_WARN("Out of range");
    return;
  }
  
  // Simultaneous close: Application invoked Close() when we are processing this FIN packet
  if (m_state == MptcpMetaFinWait1)
  {
    NS_LOG_DEBUG ("FIN_WAIT_1 -> CLOSING");
    m_state = MptcpMetaClosing;
    
    //Send a Data Ack
    SendDataAck(sf);
    
    if (m_txBuffer->Size () == 0 && (dack == m_highTxMark + SequenceNumber32 (1)))
    {
      CloseAllSubflows();
      TimeWait();
    }
  }
  else if (m_state == MptcpMetaFinWait2)
  {
    //Send a Data Ack
    SendDataAck(sf);
    TimeWait();
  }
  else
  {
    DoPeerClose (sf); // Change state, respond with ACK
  }
  
}
  
/** Received a in-sequence FIN. Close down this socket. */
// DATA FIN is in sequence, notify app and respond with a DATA ACK
void
MpTcpMetaSocket::DoPeerClose(Ptr<MpTcpSubflow> sf)
{
  NS_ASSERT (m_state == MptcpMetaEstablished || (m_state == MptcpMetaPreEstablished && sf->m_state == SYN_RCVD));
  
  // Move the state to CLOSE_WAIT
  NS_LOG_DEBUG (TcpStateName[m_state] << " -> CLOSE_WAIT");
  m_state = MptcpMetaCloseWait;
  
  if (!m_tcpParams->m_closeNotified)
  {
    // The normal behaviour for an application is that, when the peer sent a in-sequence
    // FIN, the app should prepare to close. The app has two choices at this point: either
    // respond with ShutdownSend() call to declare that it has nothing more to send and
    // the socket can be closed immediately; or remember the peer's close request, wait
    // until all its existing data are pushed into the TCP socket, then call Close()
    // explicitly.
    NS_LOG_LOGIC ("TCP " << this << " calling NotifyNormalClose");
    NotifyNormalClose ();
    m_tcpParams->m_closeNotified = true;
  }
  if (m_tcpParams->m_shutdownSend)
  { // The application declares that it would not sent any more, close this socket
    Close ();
  }
  else if (m_state == MptcpMetaCloseWait) //The state could have changed due to the notify close callback
  { // Need to Data ACK, the application will close later
    SendDataAck(sf);
  }
  if (m_state == MptcpMetaLastAck)
  {
    NS_LOG_LOGIC ("TcpSocketBase " << this << " scheduling Last Ack Timeout");
    Time lastRto = sf->ComputeRTO();
    m_lastAckEvent = Simulator::Schedule (lastRto, &MpTcpMetaSocket::LastAckTimeout, this);
  }
}


/** Do the action to close the socket. Usually send a packet with appropriate
 flags depended on the current m_state.

 TODO use a closeAndNotify in more situations
 */
int
MpTcpMetaSocket::DoClose()
{
  NS_LOG_FUNCTION(this << " in state " << TcpStateName[m_state]);

  switch (m_state)
  {
    case MptcpMetaEstablished:
      // send FIN to close the peer
    {
      NS_LOG_INFO ("ESTABLISHED -> FIN_WAIT_1");
      
      m_state = MptcpMetaFinWait1;
      SendDataFin(false);
      break;
    }
    
    case MptcpMetaCloseWait:
    {
      // send ACK to close the peer
      NS_LOG_INFO ("CLOSE_WAIT -> LAST_ACK");
      m_state = MptcpMetaLastAck;
      SendDataFin(true);
      break;
    }
    case MptcpMetaPreEstablished:
    {
      if (m_master->GetState() == SYN_RCVD)
      {
        m_state = MptcpMetaFinWait1;
        SendDataFin(false);
      }
      else if (m_master->GetState() == SYN_SENT)
      {
        //TODO: fast close
        //      SendRST();
        CloseAndNotify();
      }
      else
      {
        NS_FATAL_ERROR("Unexpected master subflow state");
      }
    }
      
    case MptcpMetaClosing:
    {
      // Send RST if application closes in SYN_SENT and CLOSING
      // TODO deallocate all childrne
      NS_LOG_WARN("trying to close while closing..");
      //      NS_LOG_INFO ("CLOSING -> LAST_ACK");
      //      m_state = TIME_WAIT;
      //        NotifyErrorClose();
      //      DeallocateEndPoint();
      
      break;
    }
    case MptcpMetaLastAck:
    case MptcpMetaListening:
    {
      CloseAndNotify();
      break;
    }
    case MptcpMetaClosed:
    case MptcpMetaFinWait1:
    case MptcpMetaFinWait2:
    case MptcpMetaTimeWait:
    default: /* mute compiler */
      // Do nothing in these four states
      break;
  }
  return 0;
}

void
MpTcpMetaSocket::CloseAllSubflows()
{
  NS_LOG_FUNCTION(this << "Closing all subflows");
  
  NS_ASSERT(m_state == MptcpMetaFinWait2 || m_state == MptcpMetaClosing || m_state == MptcpMetaLastAck);
  
  for(SubflowList::iterator it = m_subflows.begin(); it != m_subflows.end(); ++it)
  {
    Ptr<MpTcpSubflow> sf = *it;
    sf->Close();
  }
}
  
bool MpTcpMetaSocket::CheckAndAppendDataFin (Ptr<MpTcpSubflow> subflow,
                                             SequenceNumber32 ssn,
                                             uint32_t length, Ptr<MpTcpMapping> mapping)
{
  //Map the ssn to a dsn
  SequenceNumber64 dsn = mapping->GetDSNFromSSN(ssn);
  //If we have no remaining data to send, set the DATA_FIN flag
  uint32_t remainingData = m_txBuffer->SizeFromSequence(dsn + length);
  if (m_tcpParams->m_closeOnEmpty && (remainingData == 0))
  {
    subflow->AppendDSSFin();
    
    if (m_state == MptcpMetaEstablished)
    { // On active close: I am the first one to send FIN
      NS_LOG_DEBUG ("ESTABLISHED -> FIN_WAIT_1");
      m_state = MptcpMetaFinWait1;
    }
    else if (m_state == MptcpMetaCloseWait)
    { // On passive close: Peer sent me FIN already
      NS_LOG_DEBUG ("CLOSE_WAIT -> LAST_ACK");
      m_state = MptcpMetaLastAck;
    }
    return true;
  }
  
  return false;
}


/** Kill this socket. This is a callback function configured to m_endpoint in
 SetupCallback(), invoked when the endpoint is destroyed. */
void
MpTcpMetaSocket::Destroy(void)
{
  NS_LOG_FUNCTION(this);
  NS_LOG_INFO("Enter Destroy(" << this << ") m_sockets: )");

  NS_LOG_ERROR("Before unsetting endpoint, check it's not used by subflow ?");
  //m_endPoint = 0;
  
  
  // TODO loop through subflows and Destroy them too ?
//  if (m_tcp != 0)
//    {
//      vector<Ptr<TcpSocketBase> >::iterator it = find(m_tcp->m_sockets.begin(), m_tcp->m_sockets.end(), this);
//      if (it != m_tcp->m_sockets.end())
//        {
//          m_tcp->m_sockets.erase(it);
//        }
//    }
//  CancelAllSubflowTimers();
//  NS_LOG_INFO("Leave Destroy(" << this << ") m_sockets:  " << m_tcp->m_sockets.size()<< ")");
}
  
void
MpTcpMetaSocket::CancelAllEvents ()
{
  m_retxEvent.Cancel ();
  m_lastAckEvent.Cancel ();
  m_timeWaitEvent.Cancel ();
  m_sendPendingDataEvent.Cancel ();
}

/* Below are the attribute get/set functions */
void
MpTcpMetaSocket::NotifyRcvBufferChange (uint32_t oldSize, uint32_t newSize)
{
 
  /* The size has (manually) increased. Actively inform the other end to prevent
   * stale zero-window states.
   */
  if (oldSize < newSize && m_connected)
  {
    NS_FATAL_ERROR("TODO:implement");
    //SendEmptyPacket (TcpHeader::ACK);
  }
}

void
MpTcpMetaSocket::SetSegSize (uint32_t size)
{
  NS_LOG_FUNCTION (this << size);
  m_segmentSize = size;
}

uint32_t
MpTcpMetaSocket::GetSegSize (void) const
{
  return m_segmentSize;
}
  
void
MpTcpMetaSocket::SetInitialSSThresh (uint32_t threshold)
{
  m_initialSsThresh = threshold;
}

uint32_t
MpTcpMetaSocket::GetInitialSSThresh (void) const
{
  return m_initialSsThresh;
}

void
MpTcpMetaSocket::SetInitialCwnd (uint32_t cwnd)
{
  m_initialCWnd = cwnd;
}

uint32_t
MpTcpMetaSocket::GetInitialCwnd (void) const
{
  return m_initialCWnd;
}
  
/** Inherited from Socket class: Bind socket to an end-point in MpTcpL4Protocol
 */
int
MpTcpMetaSocket::Bind()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT(m_master);
  return m_master->Bind();
}

/** Inherit from socket class: Bind socket (with specific address) to an end-point in TcpL4Protocol
 convert to noop
 */
int
MpTcpMetaSocket::Bind(const Address &address)
{
  NS_LOG_FUNCTION (this<<address);
  NS_ASSERT(m_master);
  return m_master->Bind(address);
}

int
MpTcpMetaSocket::Bind6()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT(m_master);
  return m_master->Bind6();
}

int
MpTcpMetaSocket::Connect(const Address & toAddress)
{
  NS_LOG_FUNCTION(this);
  NS_ASSERT(m_master);
  return m_master->Connect(toAddress);
}
  
int
MpTcpMetaSocket::Listen(void)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT(m_master);
  return m_master->Listen();
  
}
  
/** Inherit from Socket class: Kill this socket and signal the peer (if any) */
int
MpTcpMetaSocket::Close(void)
{
  NS_LOG_FUNCTION (this);
  
  /// \internal
  /// First we check to see if there is any unread rx data.
  /// \bugid{426} claims we should send reset in this case.
  if (m_rxBuffer->Size () != 0)
  {
    NS_LOG_WARN ("Socket " << this << " << unread rx data during close.  Sending reset." <<
                 "This is probably due to a bad sink application; check its code");
     NS_FATAL_ERROR("TODO rxbuffer != 0, should send fast close");
    //SendRST ();
    return 0;
  }
  
  if (m_txBuffer->SizeFromSequence (m_nextTxSequence) > 0)
  { // App close with pending data must wait until all data transmitted
    if (m_tcpParams->m_closeOnEmpty == false)
    {
      m_tcpParams->m_closeOnEmpty = true;
      NS_LOG_INFO ("Socket " << this << " deferring close, state " << TcpStateName[m_state]);
    }
    return 0;
  }
  return DoClose ();
}
  
int MpTcpMetaSocket::ShutdownSend (void)
{
  //this prevents data from being added to the buffer
  m_tcpParams->m_shutdownSend = true;
  m_tcpParams->m_closeOnEmpty = true;
  //if buffer is already empty, send a fin now
  //otherwise fin will go when buffer empties.
  if (m_txBuffer->Size () == 0)
  {
    if (m_state == MptcpMetaEstablished || m_state == MptcpMetaCloseWait)
    {
      NS_LOG_INFO ("Emtpy tx buffer, send fin");
      SendDataFin(false);
      
      if (m_state == MptcpMetaEstablished)
      { // On active close: I am the first one to send FIN
        NS_LOG_DEBUG ("ESTABLISHED -> FIN_WAIT_1");
        m_state = MptcpMetaFinWait1;
      }
      else
      { // On passive close: Peer sent me FIN already
        NS_LOG_DEBUG ("CLOSE_WAIT -> LAST_ACK");
        m_state = MptcpMetaLastAck;
      }
    }
  }
  
  return 0;
}

int MpTcpMetaSocket::ShutdownRecv (void)
{
  m_tcpParams->m_shutdownRecv = true;
  return 0;
}

/* Inherit from Socket class: Get the max number of bytes an app can send */
uint32_t
MpTcpMetaSocket::GetTxAvailable(void) const
{
  NS_LOG_FUNCTION (this);
  uint32_t value = m_txBuffer->Available();
  NS_LOG_DEBUG("Tx available " << value);
  return value;
}
  
/** Inherit from Socket class: Get the max number of bytes an app can read */
uint32_t
MpTcpMetaSocket::GetRxAvailable(void) const
{
  NS_LOG_FUNCTION (this);
  return m_rxBuffer->Available();
}
  
void MpTcpMetaSocket::SetSndBufSize (uint32_t size)
{
  NS_LOG_FUNCTION (this << size);
  m_txBuffer->SetMaxBufferSize (size);
}

uint32_t MpTcpMetaSocket::GetSndBufSize (void) const
{
  return m_txBuffer->MaxBufferSize ();
}

void MpTcpMetaSocket::SetRcvBufSize (uint32_t size)
{
  NS_LOG_FUNCTION (this << size);
  uint32_t oldSize = GetRcvBufSize ();
  
  m_rxBuffer->SetMaxBufferSize (size);
  
  NotifyRcvBufferChange(oldSize, size);
}

uint32_t MpTcpMetaSocket::GetRcvBufSize (void) const
{
  return m_rxBuffer->MaxBufferSize ();
}
  
Ptr<TcpTxBuffer64> MpTcpMetaSocket::GetTxBuffer (void) const
{
  return m_txBuffer;
}

Ptr<TcpRxBuffer64> MpTcpMetaSocket::GetRxBuffer (void) const
{
  return m_rxBuffer;
}
  
int MpTcpMetaSocket::GetSockName (Address &address) const
{
  return m_master->GetSockName(address);
}
  
int MpTcpMetaSocket::GetPeerName (Address &address) const
{
  return m_master->GetPeerName(address);
}
  
Ptr<Packet>
MpTcpMetaSocket::Recv(uint32_t maxSize, uint32_t flags)
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_MSG_IF (flags, "use of flags is not supported in TcpSocketBase::Recv()");
  
  if (m_rxBuffer->Size () == 0 && m_state == MptcpMetaCloseWait)
  {
    return Create<Packet> (); // Send EOF on connection close
  }
  
  Ptr<Packet> outPacket = m_rxBuffer->Extract (maxSize);
  return outPacket;
}

Ptr<Packet>
MpTcpMetaSocket::RecvFrom (uint32_t maxSize, uint32_t flags, Address &fromAddress)
{
  NS_LOG_FUNCTION (this << maxSize << flags);
  Ptr<Packet> packet = Recv (maxSize, flags);
  // Null packet means no data to read, and an empty packet indicates EOF
  if (packet != 0 && packet->GetSize () != 0)
  {
    Ipv4EndPoint* endPoint = m_master->GetEndpoint();
    Ipv6EndPoint* endPoint6 = m_master->GetEndpoint6();
    
    if (endPoint != 0)
    {
      fromAddress = InetSocketAddress (endPoint->GetPeerAddress (), endPoint->GetPeerPort ());
    }
    else if (endPoint6 != 0)
    {
      fromAddress = Inet6SocketAddress (endPoint6->GetPeerAddress (), endPoint6->GetPeerPort ());
    }
    else
    {
      fromAddress = InetSocketAddress (Ipv4Address::GetZero (), 0);
    }
  }
  return packet;
}
  
// in fact it just calls SendPendingData()
int
MpTcpMetaSocket::Send(Ptr<Packet> p, uint32_t flags)
{
  NS_LOG_FUNCTION (this << p);
  NS_ABORT_MSG_IF (flags, "use of flags is not supported in MpTcpMetaSocket::Send()");
  
  TcpSocket::TcpStates_t masterState = m_master->GetState();
  
  if (m_state == MptcpMetaEstablished || m_state == MptcpMetaPreEstablished)
  {
    // Store the packet into Tx buffer
    if (!m_txBuffer->Add (p))
    { // TxBuffer overflow, send failed
      m_errno = ERROR_MSGSIZE;
      return -1;
    }
    if (m_tcpParams->m_shutdownSend)
    {
      m_errno = ERROR_SHUTDOWN;
      return -1;
    }
    // Submit the data to lower layers
    NS_LOG_LOGIC ("txBufSize=" << m_txBuffer->Size () << " state " << TcpStateName[masterState]);
    if (m_state == MptcpMetaEstablished)
    { // Try to send the data out
      if (!m_sendPendingDataEvent.IsRunning ())
      {
        m_sendPendingDataEvent = Simulator::Schedule (TimeStep (1),
                                                      &MpTcpMetaSocket::SendPendingData,
                                                      this);
      }
    }
    // This does not match the return type uint32_t != int
    return p->GetSize ();
  }
  else
  { // Connection not established yet
    m_errno = ERROR_NOTCONN;
    return -1; // Send failure
  }
}

/* Inherit from Socket class: In MpTcpMetaSocket, it is same as Send() call */
int
MpTcpMetaSocket::SendTo (Ptr<Packet> p, uint32_t flags, const Address &address)
{
  return Send (p, flags); // SendTo() and Send() are the same
}
  
Ptr<TcpSocketImpl> MpTcpMetaSocket::Fork (void)
{
  Ptr<MpTcpMetaSocket> socket = CopyObject<MpTcpMetaSocket>(this);
  return socket;
}
  
Ptr<TcpSocketImpl> MpTcpMetaSocket::Fork (Ptr<MpTcpSubflow> subflow)
{
  Ptr<MpTcpMetaSocket> socket = CopyObject<MpTcpMetaSocket>(this);
  Ptr<MpTcpSubflow> master = CopyObject<MpTcpSubflow>(subflow);
  socket->AddSubflow(master, true);
  master->SetMeta(socket);
  return socket;
}
  
void MpTcpMetaSocket::CompleteFork(Ptr<Packet> p,
                                   const TcpHeader& h,
                                   const Address& fromAddress,
                                   const Address& toAddress)
{
  // Set the receieve buffer correctly
  m_rxBuffer->SetNextRxSequence (SequenceNumber32 (1));
  
  m_master->CompleteFork(p, h, fromAddress, toAddress);
}
  
void MpTcpMetaSocket::SetMptcpEnabled (bool flag)
{
  //Does nothing, mptcp should always be enabled
}

}  //namespace ns3
