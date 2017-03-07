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
  if (m_node) { std::clog << Simulator::Now ().GetSeconds () << " [node " << m_node->GetId () << ": ] "; }
//<< TcpStateName[m_node->GetTcp()->GetState()] <<

#include <iostream>
#include <cmath>
#include "mptcp-mapping.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/abort.h"
#include "mptcp-subflow.h"
#include "tcp-socket-base.h"
#include "tcp-l4-protocol.h"
#include "ns3/ipv4-address.h"
#include "ipv4-end-point.h"
#include "ipv6-end-point.h" // it is not exported in ns3.19
#include "ipv4.h"
#include "ipv6.h"
#include "ns3/node.h"
#include "ns3/ptr.h"
#include "tcp-option-mptcp.h"
#include "mptcp-id-manager.h"
//#include "ns3/ipv4-address.h"
#include "ns3/trace-helper.h"
#include <algorithm>
//#include <openssl/sha.h>



/*
#define DISABLE_MEMBER(retType,member) retType \
                                        MpTcpSubflow::member(void) {\
                                            NS_FATAL_ERROR("This should never be called. The meta will make the subflow pass from LISTEN to ESTABLISHED."); \
                                        }
*/

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("MpTcpSubflow");

NS_OBJECT_ENSURE_REGISTERED(MpTcpSubflow);


TypeId
MpTcpSubflow::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::MpTcpSubflow")
      .SetParent<TcpSocketBase>()
      .SetGroupName ("Internet")
      .AddConstructor<MpTcpSubflow>()
    ;
  return tid;
}

TypeId
MpTcpSubflow::GetInstanceTypeId(void) const
{
  return MpTcpSubflow::GetTypeId();
}


void
MpTcpSubflow::SetMeta(Ptr<MpTcpMetaSocket> metaSocket)
{
  NS_ASSERT(metaSocket);
  NS_LOG_FUNCTION(this);
  m_metaSocket = metaSocket;
  m_tcb->m_socket = metaSocket;
}

void MpTcpSubflow::SetMptcpEnabled (bool flag)
{
  //does nothing
}
  
void
MpTcpSubflow::DumpInfo() const
{
      NS_LOG_LOGIC ("MpTcpSubflow " << this << " SendPendingData" <<
//          " w " << w <<
          " rxwin " << m_rWnd <<
//          " segsize " << GetSegSize() <<
          " nextTxSeq " << m_tcb->m_nextTxSequence <<
          " highestRxAck " << m_txBuffer->HeadSequence() <<
//          " pd->Size " << m_txBuffer->Size () <<
          " pd->SFS " << m_txBuffer->SizeFromSequence (m_tcb->m_nextTxSequence)
          );
}


/*
DupAck
RFC 6824
"As discussed earlier, however, an MPTCP
   implementation MUST NOT treat duplicate ACKs with any MPTCP option,
   with the exception of the DSS option, as indications of congestion
   [12], and an MPTCP implementation SHOULD NOT send more than two
   duplicate ACKs in a row for signaling purposes."
*/
//void
//MpTcpSubflow::DupAck(const TcpHeader& t, uint32_t count)


// TODO check with parent's
void
MpTcpSubflow::CancelAllTimers()
{
  NS_LOG_FUNCTION(this);
  //(int) sFlowIdx
  m_retxEvent.Cancel();
  m_lastAckEvent.Cancel();
  m_timewaitEvent.Cancel();
  NS_LOG_LOGIC( "CancelAllTimers");
}

// TODO remove in favor
int
MpTcpSubflow::DoConnect()
{
  NS_LOG_FUNCTION (this);
  return TcpSocketBase::DoConnect();
}


/** Inherit from Socket class: Kill this socket and signal the peer (if any) */
int
MpTcpSubflow::Close(void)
{
  NS_LOG_FUNCTION (this);

  return TcpSocketBase::Close();
}


// Does this constructor even make sense ? no ? to remove ?
MpTcpSubflow::MpTcpSubflow(const MpTcpSubflow& sock)
  : TcpSocketBase(sock),
  m_masterSocket(false),  //!always set to false, should be explicitly set later
  m_localNonce(sock.m_localNonce),
  m_id(0),
  m_dssFlags(0),
  m_routeId(0),
  m_metaSocket(0),
  m_backupSubflow(sock.m_backupSubflow)
{
  NS_LOG_FUNCTION (this << &sock);
  NS_LOG_LOGIC ("Invoked the copy constructor");
}

MpTcpSubflow::MpTcpSubflow () :
    TcpSocketBase(),
    m_routeId(0),
    m_metaSocket(0),
    m_backupSubflow(false),
    m_masterSocket(false),
    m_localNonce(0),
    m_id(0),
    m_dssFlags(0)
{
  NS_LOG_FUNCTION(this);
}

MpTcpSubflow::~MpTcpSubflow()
{
  NS_LOG_FUNCTION(this);
}


/**
TODO maybe override that not to have the callbacks
**/
void
MpTcpSubflow::CloseAndNotify(void)
{
  //TODO
  NS_LOG_FUNCTION_NOARGS();
  TcpSocketBase::CloseAndNotify();
  GetMeta()->OnSubflowClosed( this, false );
}


/**
Mapping should already exist when sending the packet
**/
int
MpTcpSubflow::Send(Ptr<Packet> p, uint32_t flags)
{
  NS_LOG_FUNCTION(this);
  NS_ASSERT(m_state == ESTABLISHED || m_state == CLOSE_WAIT);
  
  // Store the packet into Tx buffer
  if (!m_txBuffer->Add (p))
  { // TxBuffer overflow, send failed
    NS_FATAL_ERROR("There should always be enough space in the subflow tx buffer");
    m_errno = ERROR_MSGSIZE;
    return -1;
  }

  //Call send pending immediately, we need to keep the nextTxSequence variable in sync with the MetaSocket
  SendPendingData(m_connected);
  

// Check that the packet is covered by mapping (TODO: remove)
  
    SequenceNumber32 ssnHead = m_txBuffer->TailSequence() - p->GetSize();
    Ptr<MpTcpMapping> temp = m_TxMappings.GetMappingForSSN(ssnHead);
    NS_ASSERT(temp);

  return 0;
}

void
MpTcpSubflow::SendEmptyPacket(uint8_t flags)
{
  NS_LOG_FUNCTION_NOARGS();
  TcpSocketBase::SendEmptyPacket(flags);
}


void
MpTcpSubflow::SendEmptyPacket(TcpHeader& header)
{
  NS_LOG_FUNCTION(this << header);


  TcpSocketBase::SendEmptyPacket(header);
}

Ptr<MpTcpMapping>
MpTcpSubflow::AddLooseMapping(SequenceNumber64 dsnHead, uint16_t length)
{
  NS_LOG_LOGIC("Adding mapping with dsn=" << dsnHead << " len=" << length);
  
  Ptr<MpTcpMapping> mapping = m_TxMappings.AddMapping (dsnHead, FirstUnmappedSSN(), length);
  NS_ASSERT_MSG(mapping, "Can't add mapping: 2 mappings overlap");
  return mapping;
}

SequenceNumber32
MpTcpSubflow::FirstUnmappedSSN()
{
    NS_LOG_FUNCTION(this);
    SequenceNumber32 ssn = m_txBuffer->TailSequence();
  
  //TODO: remove firstunmappedSSN entirely, is wrong
  
    /*if(!m_TxMappings.FirstUnmappedSSN(ssn))
    {
        ssn = m_txBuffer->TailSequence();
    }*/
    return ssn;
}

// Fills the vector with all the pieces of data it can accept
// but not in TxBuffer
//                SequenceNumber64 headDsn,
//                std::vector< std::pair<SequenceNumber64, uint16_t> >& missing
void
MpTcpSubflow::GetMappedButMissingData(set<Ptr<MpTcpMapping>>& missing)
{
    //!
    NS_LOG_FUNCTION(this);
//    missing.clear();

    SequenceNumber32 startingSsn = m_txBuffer->TailSequence();

    m_TxMappings.GetMappingsStartingFromSSN(startingSsn, missing);
}

  /* We don't automatically embed mappings since we want the possibility to create mapping spanning over several segments
//   here, it should already have been put in the packet, we just check
//  that the

*/

// pass as const ref
void
MpTcpSubflow::SendPacket(TcpHeader header, Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << header <<  p);
  // TODO here we should decide if we call AppendMapping or not and with which value

  //Append the subflow tag if enabled
  if (m_metaSocket->GetTagSubflows())
  {
    MpTcpSubflowTag sfTag;
    sfTag.SetSubflowId(GetSubflowId());
    sfTag.SetSourceToken(m_metaSocket->GetLocalToken());
    sfTag.SetDestToken(m_metaSocket->GetPeerToken());
    p->AddPacketTag(sfTag);
  }
  
  //! if we send data...
  if(p->GetSize() && !IsInfiniteMappingEnabled())
  {
    //... we must decide to send a mapping or not
    // For now we always append the mapping but we could have mappings spanning over several packets.
    // and thus not add the mapping for several packets
    ///============================
    SequenceNumber32 ssnHead = header.GetSequenceNumber();
    
    Ptr<MpTcpMapping> mapping = m_TxMappings.GetMappingForSSN(ssnHead);
    if(!mapping)
    {
      m_TxMappings.Dump();
      NS_FATAL_ERROR("Could not find mapping associated to ssn");
    }
    NS_ASSERT_MSG(mapping->TailSSN() >= ssnHead +p->GetSize() -1, "mapping should cover the whole packet" );
    
    AppendDSSMapping(mapping);
    // For now we append the data ack everytime
    AppendDSSAck();
    
    //Check to see if we need to add the DATA_FIN option, i.e. this is the last packet and close on empty is true.
    GetMeta()->CheckAndAppendDataFin(this, ssnHead, p->GetSize(), mapping);

  }
  
  TcpSocketBase::SendPacket(header, p);
  m_dssFlags = 0; // reset for next packet
}

/**
 *
 */
uint32_t
MpTcpSubflow::SendDataPacket(TcpHeader& header, SequenceNumber32 ssnHead, uint32_t maxSize)
{
  NS_LOG_FUNCTION(this << "Sending packet starting at SSN [" << ssnHead.GetValue() << "] with len=" << maxSize);
  //! if we send data...
//  if(p->GetSize() && !IsInfiniteMappingEnabled())
//  {

      Ptr<MpTcpMapping> mapping = m_TxMappings.GetMappingForSSN(ssnHead);
      if(!mapping)
      {
        m_TxMappings.Dump();
        NS_FATAL_ERROR("Could not find mapping associated to ssn");
      }
  
  // Here we set the maxsize to the size of the mapping
  return TcpSocketBase::SendDataPacket(header, ssnHead, std::min(maxSize, (uint32_t)mapping->GetLength()));
}


bool
MpTcpSubflow::IsInfiniteMappingEnabled() const
{
    return GetMeta()->IsInfiniteMappingEnabled();
}

/*
behavior should be the same as in TcpSocketBase
TODO check if m_cWnd is

*/
void
MpTcpSubflow::Retransmit(void)
{
  NS_LOG_FUNCTION (this);
  TcpSocketBase::Retransmit();
}


// TODO this could be replaced
void
MpTcpSubflow::DoRetransmit()
{
  NS_LOG_FUNCTION(this);

  GetMeta()->OnSubflowRetransmit(this);

  TcpSocketBase::DoRetransmit();
}

/**
Received a packet upon LISTEN state.
*/
void
MpTcpSubflow::ProcessListen(Ptr<Packet> packet, const TcpHeader& tcpHeader, const Address& fromAddress, const Address& toAddress)
{
  NS_LOG_FUNCTION (this << tcpHeader);
  
  // Extract the flags. PSH and URG are not honoured.
  uint8_t tcpflags = tcpHeader.GetFlags() & ~(TcpHeader::PSH | TcpHeader::URG);
  
  //Check to see if this is a SYN, and whether it has the MP_JOIN option
  Ptr<const TcpOptionMpTcpMain> option = GetMptcpOptionWithSubtype(tcpHeader, TcpOptionMpTcpMain::MP_JOIN);
  if (option && (tcpflags & TcpHeader::SYN))
  {
    Ptr<const TcpOptionMpTcpJoin> joinOption = DynamicCast<const TcpOptionMpTcpJoin>(option);
    NS_ASSERT(joinOption);
    
    //Get the associated meta socket from the join token
    Ptr<MpTcpMetaSocket> meta = m_tcp->LookupMpTcpToken(joinOption->GetPeerToken());
    if(meta)
    {
      meta->NewSubflowJoinRequest(this, packet, tcpHeader, fromAddress, toAddress, joinOption);
    }
    else
    {
      //TODO received an MP_JOIN for a connection we don't know about, implement correct error handling
      NS_FATAL_ERROR("Received MP_JOIN with an unknown connection token");
    }
  }
  else
  {
    //If no join option, perform a normal socket listen.
    TcpSocketBase::ProcessListen (packet, tcpHeader, fromAddress, toAddress);
  }
}
  
void MpTcpSubflow::EstablishConnection(Ptr<Packet> packet, const TcpHeader& tcpHeader, bool withAck)
{
  TcpSocketBase::EstablishConnection(packet, tcpHeader, withAck);
  
  GetMeta()->EstablishSubflow(this, packet, tcpHeader);
}

Ptr<MpTcpMetaSocket>
MpTcpSubflow::GetMeta() const
{
  NS_ASSERT(m_metaSocket);
  return m_metaSocket;
}
  
void MpTcpSubflow::SetSubflowId (uint32_t subflowId)
{
  m_id = subflowId;
}

uint32_t MpTcpSubflow::GetSubflowId () const
{
  return m_id;
}

/*
It is also encouraged to
   reduce the timeouts (Maximum Segment Life) on subflows at end hosts.
Move TCP to Time_Wait state and schedule a transition to Closed state
*/
void
MpTcpSubflow::TimeWait()
{
  NS_LOG_INFO (TcpStateName[m_state] << " -> TIME_WAIT");
  m_state = TIME_WAIT;
  CancelAllTimers();
  // Move from TIME_WAIT to CLOSED after 2*MSL. Max segment lifetime is 2 min
  // according to RFC793, p.28
  m_timewaitEvent = Simulator::Schedule(Seconds(m_tcpParams->m_msl), &MpTcpSubflow::CloseAndNotify, this);
}

void
MpTcpSubflow::AddOptions (TcpHeader& header)
{
  NS_LOG_FUNCTION (this << header);
  
  if(IsTcpOptionAllowed(TcpOption::MPTCP))
  {
    NS_LOG_DEBUG("MPTCP enabled");
    AddMpTcpOptions(header);
  }

  //Call the superclass AddOptions
  TcpSocketBase::AddOptions(header);
}
  
void
MpTcpSubflow::AddMpTcpOptions (TcpHeader& header)
{
  NS_LOG_FUNCTION(this);

  if((header.GetFlags () & TcpHeader::SYN))
  {
    AddOptionMpTcp3WHS (header);
  }
  // as long as we've not received an ack from the peer we
  // send an MP_CAPABLE with both keys
  else if(!GetMeta()->FullyEstablished())
  {
    AddOptionMpTcp3WHS (header);
  }
  
  /// Constructs DSS if necessary
  /////////////////////////////////////////
  if(m_dssFlags)
  {
    AddMpTcpOptionDSS (header);
  }
}
  
//TcpOptionMpTcpJoin::State
void
MpTcpSubflow::AddOptionMpTcp3WHS(TcpHeader& hdr) const
{
  //NS_ASSERT(m_state == SYN_SENT || m_state == SYN_RCVD);
  NS_LOG_FUNCTION(this << hdr << hdr.FlagsToString(hdr.GetFlags()));
  
  if(IsMaster())
  {
    //! Use an MP_CAPABLE option
    Ptr<TcpOptionMpTcpCapable> mpc =  CreateObject<TcpOptionMpTcpCapable>();
    switch(hdr.GetFlags())
    {
      case TcpHeader::SYN:
      case (TcpHeader::SYN | TcpHeader::ACK):
        mpc->SetSenderKey(GetMeta()->GetLocalKey());
        break;
      case TcpHeader::ACK:
        mpc->SetSenderKey(GetMeta()->GetLocalKey());
        mpc->SetPeerKey(GetMeta()->GetPeerKey());
        break;
      default:
        NS_FATAL_ERROR("Should never happen");
        break;
    };
    NS_LOG_INFO("Appended option" << mpc);
    hdr.AppendOption( mpc );
  }
  else
  {
    Ptr<TcpOptionMpTcpJoin> join =  CreateObject<TcpOptionMpTcpJoin>();
    
    switch(hdr.GetFlags())
    {
      case TcpHeader::SYN:
      {
        join->SetMode(TcpOptionMpTcpJoin::Syn);
        join->SetPeerToken(GetMeta()->GetPeerToken());
        join->SetNonce(0);
        break;
      }
        
      case TcpHeader::ACK:
      {
        uint8_t hmac[20];
        
        join->SetMode(TcpOptionMpTcpJoin::Ack);
        join->SetHmac(hmac);
        break;
      }
        
      case (TcpHeader::SYN | TcpHeader::ACK):
      {
        join->SetMode(TcpOptionMpTcpJoin::SynAck);
        //! TODO request from idmanager an id
        static uint8_t id = 0;
        // TODO
        NS_LOG_WARN("IDs are incremental, there is no real logic behind it yet");
        //id = GetIdManager()->GetLocalAddrId( InetSocketAddress(m_endPoint->GetLocalAddress(),m_endPoint->GetLocalPort()) );
        join->SetAddressId(id++);
        join->SetTruncatedHmac(424242); // who cares
        join->SetNonce(4242); //! truly random :)
        
        break;
      }
        
      default:
      {
        NS_FATAL_ERROR("Should never happen");
        break;
      }
    }
    
    NS_LOG_INFO("Appended option" << join);
    hdr.AppendOption( join );
  }
}
  
void
MpTcpSubflow::AddMpTcpOptionDSS(TcpHeader& header)
{
  NS_LOG_FUNCTION(this);
  Ptr<TcpOptionMpTcpDSS> dss = Create<TcpOptionMpTcpDSS>();
  const bool sendDataFin = m_dssFlags &  TcpOptionMpTcpDSS::DataFin;
  const bool sendDataAck = m_dssFlags & TcpOptionMpTcpDSS::DataAckPresent;
  
  if(sendDataAck)
  {
    // TODO replace with member function to keep isolation
    uint64_t dack = GetMeta()->GetRxBuffer()->NextRxSequence().GetValue();
    //Make sure ACK is 64 bits
    dss->SetDataAck (dack, false);
  }
  
  // If no mapping set but DATA_FIN set, we have to create the mapping from scratch
  if(sendDataFin && !(m_dssFlags & TcpOptionMpTcpDSS::DSNMappingPresent))
  {
    //The ssn should be 0 for packets without any data
    m_dssMapping = Create<MpTcpMapping>(GetMeta()->GetNextTxSequence(), SequenceNumber32(0), 1);
    m_dssFlags |= TcpOptionMpTcpDSS::DSNMappingPresent;
  }
  
  // if there is a mapping to send
  if(m_dssFlags & TcpOptionMpTcpDSS::DSNMappingPresent)
  {
    dss->SetMapping(m_dssMapping->HeadDSN(), m_dssMapping->HeadSSN(),
                    m_dssMapping->GetLength(), sendDataFin);
  }
  bool success = header.AppendOption(dss);
  NS_ASSERT(success);
}


void
MpTcpSubflow::ProcessClosing(Ptr<Packet> packet, const TcpHeader& tcpHeader)
{
  NS_LOG_FUNCTION (this << tcpHeader);

  return TcpSocketBase::ProcessClosing(packet,tcpHeader);
}

/** Received a packet upon CLOSE_WAIT, FIN_WAIT_1, or FIN_WAIT_2 states */
void
MpTcpSubflow::ProcessWait(Ptr<Packet> packet, const TcpHeader& tcpHeader)
{
  NS_LOG_FUNCTION (this << tcpHeader);

  TcpSocketBase::ProcessWait(packet,tcpHeader);
}

/** Deallocate the end point and cancel all the timers */
void
MpTcpSubflow::DeallocateEndPoint(void)
{
    NS_LOG_FUNCTION(this);
    TcpSocketBase::DeallocateEndPoint();
}

Ptr<TcpSocketImpl> MpTcpSubflow::Fork (void)
{
  //we would like to fork the MetaSocket really
  return GetMeta()->Fork(this);
}
  
void
MpTcpSubflow::CompleteFork(Ptr<Packet> p, const TcpHeader& h, const Address& fromAddress, const Address& toAddress)
{
  NS_LOG_INFO( this << "Completing fork of MPTCP subflow");

  // Get port and address from peer (connecting host)
  // TODO upstream ns3 should assert that to and from Address are of the same kind
  TcpSocketBase::CompleteFork(p, h, fromAddress, toAddress);
}
  

Ptr<MpTcpPathIdManager>
MpTcpSubflow::GetIdManager()
{
  return GetMeta()->m_remotePathIdManager;
}

void
MpTcpSubflow::ConnectionSucceeded(void)
{
  NS_LOG_LOGIC(this << "Connection succeeded");
  if (m_masterSocket)
  {
    m_connected = true;
    GetMeta()->ConnectionSucceeded(this);
  }
  else
  {
    TcpSocketBase::ConnectionSucceeded ();
  }
}

/** Received a packet upon SYN_SENT */
void
MpTcpSubflow::ProcessSynSent(Ptr<Packet> packet, const TcpHeader& tcpHeader)
{
  NS_LOG_FUNCTION (this << tcpHeader);
  NS_ASSERT(m_state == SYN_SENT);

  NS_LOG_DEBUG("endp=" << m_endPoint);
  TcpSocketBase::ProcessSynSent(packet, tcpHeader);
}

void
MpTcpSubflow::ProcessOptionMpTcpCapable(Ptr<const TcpOptionMpTcpMain> option)
{
  NS_LOG_LOGIC(this << option);
  NS_ASSERT_MSG(IsMaster(), "You can receive MP_CAPABLE only on the master subflow");
  
  /**
   * Here is how the MPTCP 3WHS works:
   *  o  SYN (A->B): A's Key for this connection.
   *  o  SYN/ACK (B->A): B's Key for this connection.
   *  o  ACK (A->B): A's Key followed by B's Key.
   *
   */
  // Expect an MP_CAPABLE option
  Ptr<const TcpOptionMpTcpCapable> mpcRcvd = DynamicCast<const TcpOptionMpTcpCapable>(option);
  NS_ASSERT_MSG(mpcRcvd, "There must be a MP_CAPABLE option");
  
  
  if(m_state == LISTEN || m_state == SYN_SENT)
  {
    GetMeta()->SetPeerKey(mpcRcvd->GetSenderKey());
  }
  else if (m_state == SYN_RCVD)
  {
    // TODO check it depending on the state, sanity check the peer key?
  }
  
  // TODO add it to the manager too
}


/**
       |             |   SYN + MP_JOIN(Token-B, R-A)  |
       |             |------------------------------->|
       |             |<-------------------------------|
       |             | SYN/ACK + MP_JOIN(HMAC-B, R-B) |
       |             |                                |
       |             |     ACK + MP_JOIN(HMAC-A)      |
       |             |------------------------------->|
       |             |<-------------------------------|
       |             |             ACK                |

 HMAC-A = HMAC(Key=(Key-A+Key-B), Msg=(R-A+R-B))
 HMAC-B = HMAC(Key=(Key-B+Key-A), Msg=(R-B+R-A))
  */

void
MpTcpSubflow::ProcessOptionMpTcpJoin(Ptr<const TcpOptionMpTcpMain> option)
{
  NS_LOG_FUNCTION(this << option);
  NS_LOG_DEBUG("Expecting MP_JOIN...");
  
  Ptr<const TcpOptionMpTcpJoin> join = DynamicCast<const TcpOptionMpTcpJoin>(option);
  // TODO should be less restrictive in case there is a loss
  
  NS_ASSERT_MSG( join, "There must be an MP_JOIN option in the SYN Packet" );
  // NS_ASSERT_MSG( join && join->GetMode() == TcpOptionMpTcpJoin::SynAck, "the MPTCP join option received is not of the expected 1 out of 3 MP_JOIN types." );
  
  uint8_t addressId = join->GetAddressId(); //!< each mptcp subflow has a uid assigned
  // TODO Here we should check the tokens
  //        uint8_t buf[20] =
  //        opt3->GetTruncatedHmac();
  NS_LOG_DEBUG("Id manager");
  GetIdManager()->AddRemoteAddr(addressId, m_endPoint->GetPeerAddress(), m_endPoint->GetPeerPort());
}


void
MpTcpSubflow::PreProcessOption (Ptr<const TcpOption> option)
{
  NS_LOG_FUNCTION(option);

  if(option->GetKind() != TcpOption::MPTCP)
  {
    return;
  }

  Ptr<const TcpOptionMpTcpMain> main = DynamicCast<const TcpOptionMpTcpMain>(option);
  switch(main->GetSubType())
  {
    case TcpOptionMpTcpMain::MP_CAPABLE:
    {
      ProcessOptionMpTcpCapable(main);
      break;
    }
      
    case TcpOptionMpTcpMain::MP_JOIN:
    {
      ProcessOptionMpTcpJoin(main);
      break;
    }
      
    case TcpOptionMpTcpMain::MP_DSS:
    {
      Ptr<const TcpOptionMpTcpDSS> dss = DynamicCast<const TcpOptionMpTcpDSS>(option);
      PreProcessOptionMpTcpDSS(dss);
      break;
    }
      
    case TcpOptionMpTcpMain::MP_FASTCLOSE:
    case TcpOptionMpTcpMain::MP_FAIL:
    default:
      NS_FATAL_ERROR("Unsupported yet");
      break;
  };
}
  
void
MpTcpSubflow::PostProcessOption (Ptr<const TcpOption> option)
{
  NS_LOG_FUNCTION(option);
  
  if(option->GetKind() != TcpOption::MPTCP)
  {
    return;
  }
  
  Ptr<const TcpOptionMpTcpMain> main = DynamicCast<const TcpOptionMpTcpMain>(option);
  if (main->GetSubType() == TcpOptionMpTcpMain::MP_DSS)
  {
      Ptr<const TcpOptionMpTcpDSS> dss = DynamicCast<const TcpOptionMpTcpDSS>(option);
      NS_ASSERT(dss);
      // Update later on
      PostProcessOptionMpTcpDSS(dss);
  }
}

int
MpTcpSubflow::Listen(void)
{
  return TcpSocketBase::Listen();
}

void
MpTcpSubflow::ProcessSynRcvd(Ptr<Packet> packet, const TcpHeader& tcpHeader, const Address& fromAddress,
    const Address& toAddress)
{
  //!
  NS_LOG_FUNCTION (this << tcpHeader);
  TcpSocketBase::ProcessSynRcvd(packet, tcpHeader, fromAddress, toAddress);
}
  
bool MpTcpSubflow::CanSendPendingData (uint32_t transmitWindow)
{
  NS_LOG_FUNCTION (this);
  
  if (m_state != ESTABLISHED && m_state != CLOSE_WAIT)
  {
    return false;
  }
  
  return TcpSocketBase::CanSendPendingData(transmitWindow);
}

  
bool
MpTcpSubflow::SendPendingData(bool withAck)
{
  //!
  NS_LOG_FUNCTION(this);
  return TcpSocketBase::SendPendingData(withAck);
}

bool
MpTcpSubflow::IsMaster() const
{
  return m_masterSocket;
}
  
void MpTcpSubflow::SetMaster ()
{
  m_masterSocket = true;
}

bool
MpTcpSubflow::BackupSubflow() const
{
  return m_backupSubflow;
}


/**
should be able to advertise several in one packet if enough space
It is possible
http://tools.ietf.org/html/rfc6824#section-3.4.1
   A host can send an ADD_ADDR message with an already assigned Address
   ID, but the Address MUST be the same as previously assigned to this
   Address ID, and the Port MUST be different from one already in use
   for this Address ID.  If these conditions are not met, the receiver
   SHOULD silently ignore the ADD_ADDR.  A host wishing to replace an
   existing Address ID MUST first remove the existing one
   (Section 3.4.2).

   A host that receives an ADD_ADDR but finds a connection set up to
   that IP address and port number is unsuccessful SHOULD NOT perform
   further connection attempts to this address/port combination for this
   connection.  A sender that wants to trigger a new incoming connection
   attempt on a previously advertised address/port combination can
   therefore refresh ADD_ADDR information by sending the option again.

**/
void
MpTcpSubflow::AdvertiseAddress(Ipv4Address addr, uint16_t port)
{
  NS_LOG_FUNCTION("Started advertising address");
//  NS_ASSERT( );
#if 0
//      IPv4Address;;ConvertFrom ( addr );
  Ptr<TcpOptionMpTcpAddAddress> addAddrOption = CreateObject<TcpOptionMpTcpAddAddress>();
  addAddrOption->SetAddress( InetSocketAddress( m_endPoint->GetLocalAddress(),0), addrId );


//      this->SendPacket(pkt, header, m_localAddress, m_remoteAddress, FindOutputNetDevice(m_localAddress) );
  NS_LOG_INFO("Advertise  Addresses-> "<< header);
  #endif
}


bool
MpTcpSubflow::StopAdvertisingAddress(Ipv4Address address)
{
  return true;
}


void
MpTcpSubflow::ReTxTimeout()
{
  NS_LOG_LOGIC("MpTcpSubflow ReTxTimeout expired !");
  TcpSocketBase::ReTxTimeout();
}

bool
MpTcpSubflow::UpdateWindowSize(const TcpHeader& header)
{
  bool updated = TcpSocketBase::UpdateWindowSize(header);
  if(updated)
  {
    /*
     * From RFC 6824:
     * The receive window is shared by all subflows and is relative to the Data
     * ACK.  Because of this, an implementation MUST NOT use the RCV.WND
     * field of a TCP segment at the connection level if it does not also
     * carry a DSS option with a Data ACK field.
     */
    
    //Note: if the connection is not established, we must consider the RCV.WND field of the SYN/SYN-ACK
    //exchange in order to correctly initialize the connection level Rwnd.
    if (!GetMeta()->FullyEstablished())
    {
      GetMeta()->UpdateWindowSize(m_rWnd);
    }
    else
    {
      Ptr<const TcpOptionMpTcpMain> option = GetMptcpOptionWithSubtype(header, TcpOptionMpTcpMain::MP_DSS);
      if(option)
      {
        Ptr<const TcpOptionMpTcpDSS> dssOption = StaticCast<const TcpOptionMpTcpDSS>(option);
        if(dssOption->GetFlags() & TcpOptionMpTcpDSS::DataAckPresent)
        {
          GetMeta()->UpdateWindowSize(m_rWnd);
        }
      }
    }
  }
  return updated;
}

void
MpTcpSubflow::UpdateTxBuffer(SequenceNumber32 ack)
{
  NS_LOG_FUNCTION(this);
  
  SequenceNumber32 startSeq = m_txBuffer->HeadSequence();
  TcpSocketBase::UpdateTxBuffer(ack);
  uint32_t length = m_txBuffer->HeadSequence() - startSeq;
  
  m_TxMappings.DiscardMappingsInSSNRange(startSeq, length);
}

/**
TODO check with its parent equivalent, may miss a few features
Receipt of new packet, put into Rx buffer

SlowStart and fast recovery remains untouched in MPTCP.
The reaction should be different depending on if we handle NR-SACK or not
*/

void
MpTcpSubflow::NewAck(const TcpHeader& header, bool resetRTO)
{
  NS_LOG_FUNCTION (this << header);

  TcpSocketBase::NewAck(header, resetRTO);
  
  if(header.GetFlags() & TcpHeader::SYN)
  {
    return;
  }
  
  //Check if we have a DACK and notify the meta socket
  /*Ptr<const TcpOptionMpTcpMain> main = GetMptcpOptionWithSubtype(header, TcpOptionMpTcpMain::MP_DSS);
  if (main)
  {
    Ptr<const TcpOptionMpTcpDSS> dss = DynamicCast<const TcpOptionMpTcpDSS>(main);
    NS_ASSERT(dss);
    if( dss->GetFlags() & TcpOptionMpTcpDSS::DataAckPresent)
    {
      GetMeta()->NewAck(this, dss->GetDataAck());
    }
  }*/
}
  
Ptr<const TcpOptionMpTcpMain> MpTcpSubflow::GetMptcpOptionWithSubtype (const TcpHeader& header, TcpOptionMpTcpMain::SubType subtype)
{
  if(header.HasOption(TcpOption::MPTCP))
  {
    TcpHeader::TcpOptionList options;
    header.GetOptions (options);
    
    for(TcpHeader::TcpOptionList::const_iterator it(options.begin()); it != options.end(); ++it)
    {
      Ptr<const TcpOption> option = *it;
      if(option->GetKind() == TcpOption::MPTCP)
      {
        Ptr<const TcpOptionMpTcpMain> main = DynamicCast<const TcpOptionMpTcpMain>(option);
        if(main->GetSubType() == subtype)
        {
          return main;
        }
      }
    }
  }
  return nullptr;

}

Ptr<NetDevice>
MpTcpSubflow::MapIpv4ToDevice (Ipv4Address addr) const
{
  NS_LOG_DEBUG(addr);
  cout << addr << endl;
  Ptr<Ipv4> ipv4client = m_node->GetObject<Ipv4>();

  for (uint32_t n = 0; n < ipv4client->GetNInterfaces(); n++)
  {
    for (uint32_t a = 0; a < ipv4client->GetNAddresses(n); a++)
    {
      //NS_LOG_UNCOND( "Client addr " << n <<"/" << a << "=" << ipv4client->GetAddress(n,a));
      if(addr ==ipv4client->GetAddress(n,a).GetLocal()) {
        //NS_LOG_UNCOND("EUREKA same ip=" << addr);
        return m_node->GetDevice(n);
      }
    }
  }
  return nullptr;
}

Ptr<NetDevice>
MpTcpSubflow::MapIpv6ToDevice (Ipv6Address addr) const
{
  NS_LOG_DEBUG(addr);
  cout << addr << endl;
  Ptr<Ipv6> ipv6client = m_node->GetObject<Ipv6>();

  for (uint32_t n = 0; n < ipv6client->GetNInterfaces(); n++)
  {
    for (uint32_t a = 0; a < ipv6client->GetNAddresses(n); a++)
    {
      //NS_LOG_UNCOND( "Client addr " << n <<"/" << a << "=" << ipv4client->GetAddress(n,a));
      if(addr ==ipv6client->GetAddress(n,a).GetAddress()) {
        //NS_LOG_UNCOND("EUREKA same ip=" << addr);
        return m_node->GetDevice(n);
      }
    }
  }
  return nullptr;
}

int
MpTcpSubflow::Bind (const Address &address)
{
  NS_LOG_FUNCTION (this << address);

  int result = TcpSocketBase::Bind(address);

  if (InetSocketAddress::IsMatchingType (address) && result ==0)
  {

    Ptr<NetDevice> dev = MapIpv4ToDevice(m_endPoint->GetLocalAddress());

    if(dev) {
      m_endPoint->BindToNetDevice(dev);
      m_boundnetdevice = dev;
    }
  }
  else if (Inet6SocketAddress::IsMatchingType (address))
  {
    Ptr<NetDevice> dev = MapIpv6ToDevice(m_endPoint6->GetLocalAddress());

    if(dev) {
      m_endPoint6->BindToNetDevice(dev);
      m_boundnetdevice = dev;
    }
  }

  return result;
}

int
MpTcpSubflow::Bind (void)
{
  if(IsMaster())
  {
    //For now, we cannot just bind to any address. The master socket must be bound to the address
    //of the first interface.
    Ptr<Ipv4> ipv4 = m_node->GetObject<Ipv4>();
    NS_ASSERT(ipv4->GetNInterfaces() >= 2);
    Ipv4Address address = ipv4->GetAddress(1, 0).GetLocal();

    return Bind(InetSocketAddress(address, 0));
  }
  else
  {
    NS_FATAL_ERROR("Should not call Bind without any address argument for secondary subflows.");
    return 0;
  }
}

int
MpTcpSubflow::Bind6()
{
  NS_LOG_FUNCTION (this);
  if (IsMaster())
  {
    //For now, we cannot just bind to any address. The master socket must be bound to the address
    //of the first interface.
    Ptr<Ipv6> ipv6 = m_node->GetObject<Ipv6>();
    NS_ASSERT(ipv6->GetNInterfaces() >= 2);
    Ipv6Address address = ipv6->GetAddress(1, 0).GetAddress();

    return Bind(Inet6SocketAddress(address, 0));
  }
  else
  {
    NS_FATAL_ERROR("Should not call Bind without any address argument for secondary subflows.");
    return 0;
  }
}

Ptr<Packet>
MpTcpSubflow::RecvFrom(uint32_t maxSize, uint32_t flags, Address &fromAddress)
{
  NS_FATAL_ERROR("Disabled in MPTCP. Use MpTcpMetaSocket.");
  return 0;
}

Ptr<Packet>
MpTcpSubflow::Recv(uint32_t maxSize, uint32_t flags)
{
  //!
  NS_FATAL_ERROR("Disabled in MPTCP. Use MpTcpMetaSocket");
  return 0;
}

Ptr<Packet>
MpTcpSubflow::Recv(void)
{
  //!
  NS_FATAL_ERROR("Disabled in MPTCP. Use MpTcpMetaSocket");
  return 0;
}

void
MpTcpSubflow::UpdateRxBuffer()
{
  NS_LOG_DEBUG(this);
  Ptr<Packet> p = Create<Packet>();
  
  uint32_t rxAvailable = GetRxAvailable();
  if(rxAvailable == 0)
  {
    NS_LOG_LOGIC("Nothing to extract");
    return;
  }
  else
  {
    NS_LOG_LOGIC(rxAvailable  << " Rx available");
  }
  
  // as in linux, we extract in order
  SequenceNumber32 headSSN = m_rxBuffer->HeadSequence();
  if(headSSN < m_rxBuffer->NextRxSequence())
  {
    uint32_t maxSize = m_rxBuffer->NextRxSequence() - headSSN;
    
    //Safe to delete the packets from this local RX Buffer, at this point
    //they should be in the meta socket's rx buffer
    //If the meta socket wants to retransmit packets from a different subflow on this subflow
    //they will have larger sequence numbers.
    
    NS_LOG_DEBUG("Extracting at most " << maxSize << " bytes ");
    p = m_rxBuffer->Extract(maxSize);
    
    m_RxMappings.DiscardMappingsInSSNRange(headSSN, maxSize);
  }
}


void
MpTcpSubflow::AppendDSSMapping(Ptr<const MpTcpMapping> mapping)
{
    NS_LOG_FUNCTION(this << mapping);
    m_dssFlags |= TcpOptionMpTcpDSS::DSNMappingPresent;
    m_dssMapping = Copy<MpTcpMapping>(mapping);
}

void
MpTcpSubflow::AppendDSSAck()
{
  NS_LOG_FUNCTION(this);
  //For now, to prevent running into the TCP option size limit when
  //we have both MP-CAPABLE and DSS options on a segment, disable
  //data till we stop sending the MP-CAPABLE option.
  if(GetMeta()->FullyEstablished())
  {
    m_dssFlags |= TcpOptionMpTcpDSS::DataAckPresent;
  }
}

void
MpTcpSubflow::AppendDSSFin()
{
    NS_LOG_FUNCTION(this);
    m_dssFlags |= TcpOptionMpTcpDSS::DataFin;
}
  
/**
 Add the received data to the Subflow level rx buffer, and meta socket rx buffer.
*/
  
void
MpTcpSubflow::ReceivedData(Ptr<Packet> p, const TcpHeader& tcpHeader)
{
  NS_LOG_FUNCTION (this << tcpHeader);

  Ptr<MpTcpMapping> mapping = m_RxMappings.GetMappingForSSN(tcpHeader.GetSequenceNumber());
  bool sendAck = false;


  // OutOfRange
  // If cannot find an adequate mapping, then it should [check RFC]
  if(!mapping)
  {
    m_RxMappings.Dump();
    NS_FATAL_ERROR("Could not find mapping associated ");
    return;
  }
  
  // Put into Rx buffer
  SequenceNumber32 expectedSSN = m_rxBuffer->NextRxSequence();
  
  //First, add the packet to the subflow level receive buffer. We need to do this first so that
  //ACKs sent have the correct SSN.
  if (!m_rxBuffer->Add(p, tcpHeader.GetSequenceNumber()))
  { // Insert failed: No data or RX buffer full, or a duplicate
    m_rxBuffer->Dump();
    
    //This is possibly a duplicate, discard the mapping from the rxMappings
    SequenceNumber32 ssn = tcpHeader.GetSequenceNumber();
    m_RxMappings.DiscardMappingsInSSNRange(ssn, p->GetSize());
    
    //Don't need to add to the meta socket if this is a duplicate
    //Send ACK immediately
    AppendDSSAck();
    SendEmptyPacket(TcpHeader::ACK);
    return;
  }
  
  //Always notify the meta socket to attempt to add packet to the buffer
  //Send ACK immediately if we cannot add to the receive buffer
  SequenceNumber64 expectedDSN = GetMeta()->GetRxBuffer()->NextRxSequence();
  if (!GetMeta()->AddToReceiveBuffer(this, p, tcpHeader, mapping))
  {
    AppendDSSAck();
    SendEmptyPacket(TcpHeader::ACK);
  }

  // Size() = Get the actual buffer occupancy
  if (m_rxBuffer->Size() > m_rxBuffer->Available() /* Out of order packets exist in buffer */
    || m_rxBuffer->NextRxSequence() > expectedSSN + p->GetSize() /* or we filled a gap */
    )
    { // A gap exists in the buffer, or we filled a gap: Always ACK
      sendAck = true;
    }
  else
    {
      // In-sequence packet: ACK if delayed ack count allows
      if (++m_delAckCount >= m_tcpParams->m_delAckMaxCount)
      {
        m_delAckEvent.Cancel ();
        m_delAckCount = 0;
        sendAck = true;
      }
      else if (m_delAckEvent.IsExpired ())
      {
        m_delAckEvent = Simulator::Schedule (m_tcpParams->m_delAckTimeout,
                                             &MpTcpSubflow::DelAckTimeout, this);
        NS_LOG_LOGIC (this << " scheduled delayed ACK at "
                      << (Simulator::Now () + Simulator::GetDelayLeft (m_delAckEvent)).GetSeconds ());
      }
    }
  
  //Remove in order packets from the subflow receive buffer and remove mappings
  UpdateRxBuffer();
  
  //Always notify the meta socket
  GetMeta()->OnSubflowRecv(this, p, tcpHeader, expectedDSN, mapping);
  
  if(sendAck)
  {
    AppendDSSAck();
    SendEmptyPacket(TcpHeader::ACK);
  }
}

  /*
   Receive Window:  The receive window in the TCP header indicates the
   amount of free buffer space for the whole data-level connection
   (as opposed to for this subflow) that is available at the
   receiver.  This is the same semantics as regular TCP, but to
   maintain these semantics the receive window must be interpreted at
   the sender as relative to the sequence number given in the
   DATA_ACK rather than the subflow ACK in the TCP header.  In this
   way, the original flow control role is preserved.  Note that some
   middleboxes may change the receive window, and so a host SHOULD
   use the maximum value of those recently seen on the constituent
   subflows for the connection-level receive window, and also needs
   to maintain a subflow-level window for subflow-level processing.
   */
  
uint32_t
MpTcpSubflow::Window (void) const
{
  NS_LOG_FUNCTION (this);
  //The size of the connection level (i.e. meta socket's) receive window
  //The "real" receive buffer is at the connection level.
  return std::min (GetMeta()->GetRwndSize(), m_tcb->m_cWnd.Get ());
}

// Ok
uint16_t
MpTcpSubflow::AdvertisedWindowSize(bool scale) const
{
  NS_LOG_DEBUG(this);
  NS_LOG_FUNCTION (this << scale);
  uint32_t w = GetMeta()->GetRxBuffer()->MaxBufferSize ();
  
  if (scale)
  {
    w >>= m_rcvWindShift;
  }
  if (w > m_tcpParams->m_maxWinSize)
  {
    w = m_tcpParams->m_maxWinSize;
    NS_LOG_WARN ("Adv window size truncated to " << m_tcpParams->m_maxWinSize << "; possibly to avoid overflow of the 16-bit integer");
  }
  NS_LOG_DEBUG ("Returning AdvertisedWindowSize of " << static_cast<uint16_t> (w));
  
  return static_cast<uint16_t> (w);
}

/*
Quote from rfc 6824:
    Because of this, an implementation MUST NOT use the RCV.WND
    field of a TCP segment at the connection level if it does not also
    carry a DSS option with a Data ACK field

    and in not established mode ?
    TODO
*/

void
MpTcpSubflow::PreProcessOptionMpTcpDSS(Ptr<const TcpOptionMpTcpDSS> dss)
{
  NS_LOG_FUNCTION (this << dss << " from subflow ");
  
  if(!GetMeta()->FullyEstablished() )
  {
    NS_LOG_LOGIC("First DSS received !");
    
    GetMeta()->BecomeFullyEstablished();
    
  }
  
  //! datafin case handled at the start of the function
  if( (dss->GetFlags() & TcpOptionMpTcpDSS::DSNMappingPresent) && !dss->DataFinMappingOnly() )
  {
    
    // Add peer mapping
    Ptr<MpTcpMapping> mapping = m_RxMappings.AddMapping(dss->GetDataSequenceNumber(),
                                                        dss->GetSubflowSequenceNumber(),
                                                        dss->GetMappingLength());
    if(!mapping)
    {
      //We hit this when we time out after a loss, and retransmit something which has already
      //been received.
      NS_LOG_WARN("Could not insert mapping: It already exists.");
      NS_LOG_UNCOND("Dumping Rx mappings...");
      m_RxMappings.Dump();
    }
  }
  
  if((dss->GetFlags() & TcpOptionMpTcpDSS::DataAckPresent))
  {
    //    NS_LOG_DEBUG("DataAck detected");
    GetMeta()->ReceivedAck(this, dss->GetDataAck());
  }
}
  
void MpTcpSubflow::PostProcessOptionMpTcpDSS(Ptr<const TcpOptionMpTcpDSS> dss)
{
  if (dss->GetFlags() & TcpOptionMpTcpDSS::DataFin)
  {
    NS_LOG_LOGIC("Data FIN detected " << dss->GetDataFinDSN());
    
    SequenceNumber64 dack;
    if (dss->GetFlags() & TcpOptionMpTcpDSS::DataAckPresent)
    {
      dack = dss->GetDataAck();
    }
    GetMeta()->PeerClose(SequenceNumber32(dss->GetDataFinDSN()), dack, this);
  }
}


/*
Upon ack receival we need to act depending on if it's new or not
-if it's new it may allow us to discard a mapping
-otherwise notify meta of duplicate

this is called
*/
void
MpTcpSubflow::ReceivedAck(Ptr<Packet> p, const TcpHeader& header)
{
  NS_LOG_FUNCTION (this << header);

  // if packet size > 0 then it will call ReceivedData
  TcpSocketBase::ReceivedAck(p, header );

  // By default we always append a DACK
  // We should consider more advanced schemes
  AppendDSSAck();
}

} // end of ns3
