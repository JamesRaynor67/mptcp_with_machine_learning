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
 */
#ifndef MPTCP_SUBFLOW_H
#define MPTCP_SUBFLOW_H

#include <stdint.h>
#include <vector>
#include <queue>
#include <list>
#include <set>
#include <map>
#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/sequence-number.h"
#include "ns3/rtt-estimator.h"
#include "ns3/event-id.h"
#include "ns3/packet.h"
#include "ns3/tcp-socket.h"
#include "ns3/ipv4-end-point.h"
#include "ns3/ipv4-address.h"
#include "mptcp-meta-socket.h"
#include "ns3/tcp-header.h"
#include "ns3/mptcp-mapping.h"
#include "ns3/tcp-option-mptcp.h"

using namespace std;

namespace ns3
{

class MpTcpMetaSocket;
class MpTcpPathIdManager;
class TcpOptionMpTcpDSS;
class TcpOptionMpTcpMain;

/**
 * \class MpTcpSubflow
*/
class MpTcpSubflow : public TcpSocketBase
{
public:

  /**
  the metasocket is the socket the application is talking to.
  Every subflow is linked to that socket.
  \param The metasocket it is linked to
  **/
  MpTcpSubflow();
  MpTcpSubflow(const MpTcpSubflow&);
  virtual ~MpTcpSubflow();
  
  static TypeId GetTypeId(void);
  
  virtual TypeId GetInstanceTypeId(void) const override;

  /**
   * will update the meta rwnd. Called by subflows whose
   * \return true
  */
  virtual bool UpdateWindowSize (const TcpHeader& header);

  /**
   * \param metaSocket
   */
  virtual void
  SetMeta(Ptr<MpTcpMetaSocket> metaSocket);
  
  /**
   *
   */
  Ptr<MpTcpMetaSocket> GetMeta() const;
  
  void SetSubflowId (uint32_t subflowId);
  uint32_t GetSubflowId () const;
  
  /**
   * Not implemented
   * \return false
   */
  bool IsInfiniteMappingEnabled() const;
  
  /**
   * Mapping is said "loose" because it is not tied to an SSN yet, this is the job
   * of this function: it will look for the FirstUnmappedSSN() and map the DSN to it.
   *
   * Thus you should call it with increased dsn.
   *
   * \param dsnHead
   */
  Ptr<MpTcpMapping> AddLooseMapping(SequenceNumber64 dsnHead, uint16_t length);

  /**
  \warning for prototyping purposes, we let the user free to advertise an IP that doesn't belong to the node
  (in reference to MPTCP connection agility).
  \note Maybe we should change this behavior ?
  TODO convert to Address to work with IPv6
  */
  virtual void
  AdvertiseAddress(Ipv4Address , uint16_t port);

  /**
  \brief Send a REM_ADDR for the specific address.
  \see AdvertiseAddress
  \return false if no id associated with the address which likely means it was never advertised in the first place
  */
  virtual bool
  StopAdvertisingAddress(Ipv4Address);
  
  virtual int Bind (void) override;
  virtual int Bind6 (void) override;
  virtual int Bind (const Address &address) override;

  /**
   * for debug
   */
  void
  DumpInfo() const;


  /**
  \brief
  \note A Master socket is the first to initiate the connection, thus it will use the option MP_CAPABLE
      during the 3WHS while any additionnal subflow must resort to the MP_JOIN option
  \return True if this subflow is the first (should be unique) subflow attempting to connect
  **/
  virtual bool IsMaster() const;
  
  virtual void SetMaster ();

  /**
  \return True if this subflow shall be used only when all the regular ones failed
  */
  virtual bool BackupSubflow() const;


//  virtual void
//  DoForwardUp(Ptr<Packet> packet, Ipv4Header header, uint16_t port, Ptr<Ipv4Interface> incomingInterface);
  
  virtual bool SendPendingData(bool withAck = false) override;


  /**
  Disabled for now.
  SendMapping should be used instead.
  **/
  int
  Send(Ptr<Packet> p, uint32_t flags);
  
  virtual bool CanSendPendingData (uint32_t transmitWindow) override;

  //! disabled
  Ptr<Packet> RecvFrom(uint32_t maxSize, uint32_t flags, Address &fromAddress);

  //! disabled
  Ptr<Packet> Recv(uint32_t maxSize, uint32_t flags);

  //! Disabled
  Ptr<Packet> Recv(void);

  virtual void DeallocateEndPoint(void);

  /**
  * This should
  */
  virtual void NewAck(const TcpHeader& header, bool resetRTO);

  virtual void TimeWait();

  virtual void DoRetransmit();

  virtual int Listen(void);
 
  void
  ProcessListen(Ptr<Packet> packet, const TcpHeader& tcpHeader,
                const Address& fromAddress, const Address& toAddress);

  virtual void
  ProcessSynRcvd(Ptr<Packet> packet, const TcpHeader& tcpHeader,
                 const Address& fromAddress, const Address& toAddress);

  virtual void ProcessSynSent(Ptr<Packet> packet, const TcpHeader& tcpHeader);
  virtual void ProcessWait(Ptr<Packet> packet, const TcpHeader& tcpHeader);

  virtual void UpdateTxBuffer(SequenceNumber32 ack) override;
  virtual void UpdateRxBuffer();
  
  /**
  */
  virtual void Retransmit(void);

  /**
   * Parse DSS essentially
   */
//  virtual int ProcessOptionMpTcpEstablished(const Ptr<const TcpOption> option);
  virtual void PreProcessOptionMpTcpDSS(Ptr<const TcpOptionMpTcpDSS> option);
  virtual void PostProcessOptionMpTcpDSS(Ptr<const TcpOptionMpTcpDSS> option);
  virtual void ProcessOptionMpTcpJoin(Ptr<const TcpOptionMpTcpMain> option);
  virtual void ProcessOptionMpTcpCapable(Ptr<const TcpOptionMpTcpMain> option);
//  virtual int ProcessTcpOptionMpTcpDSS(Ptr<const TcpOptionMpTcpDSS> dss);

  Ptr<MpTcpPathIdManager> GetIdManager();

  /**
  Temporary, for debug
  **/
//  void
//  SetupTracing(const std::string prefix);
//  MpTcpMapping getSegmentOfACK( uint32_t ack);
  
  /**
   * Will send MP_JOIN or MP_CAPABLE depending on if it is master or not
   * Updates the meta endpoint
   *
   * \see TcpSocketBase::CompleteFork
   */
  virtual void
  CompleteFork(Ptr<Packet> p, const TcpHeader& h, const Address& fromAddress, const Address& toAddress) override;

protected:
  friend class MpTcpMetaSocket;

  virtual void EstablishConnection(Ptr<Packet> packet, const TcpHeader& tcpHeader, bool withAck) override;

  /**
  * This is a public function in TcpSocketBase but it shouldn't be public here !
  **/
  virtual int Close(void);

  void GenerateDataPacketHeader(TcpHeader& header, SequenceNumber32 seq, uint32_t maxSize, bool withAck);


  virtual void CloseAndNotify(void);

  virtual void GetMappedButMissingData(set<Ptr<MpTcpMapping>>& missing);

  /**
   * Depending on if this subflow is master or not, we want to
   * trigger
   * Callbacks being private members
   * Overrides parent in order to warn meta
   **/
  virtual void ConnectionSucceeded(void);

  int DoConnect();

  virtual void AddOptions (TcpHeader& tcpHeader) override;
  
  /**
  * TODO in fact, instead of relying on IsMaster etc...
  * this should depend on meta's state , if it is wait or not
  * and thus it should be pushed in meta (would also remove the need for crypto accessors)
  */
  virtual void AddOptionMpTcp3WHS(TcpHeader& hdr) const;
  
  /**
   * Corresponds to mptcp_write_dss_mapping and mptcp_write_dss_ack
   */
  virtual void AddMpTcpOptionDSS(TcpHeader& header);
  
  /**
   * Overrides the TcpSocketBase that just handles the MP_CAPABLE option.
   *
   */
  virtual void AddMpTcpOptions (TcpHeader& header);

  // TO remove hopefully
  virtual void ProcessClosing(Ptr<Packet> packet, const TcpHeader& tcpHeader);
  
  /**
   * Process an option before we call the main TCP functionality on receiving a segment
   */
  virtual void PreProcessOption(Ptr<const TcpOption> option) override;
  
  /**
   * Process an option after we call the main TCP functionality on receiving a segment
   */
  virtual void PostProcessOption(Ptr<const TcpOption> option) override;
  
  // Return the max possible number of unacked bytes. Note that we take the connection level rWnd into consideration
  // not the subflow rWnd.
  virtual uint32_t Window(void) const override;
  
  /**
   \return Value advertised by the meta socket
   */
  virtual uint16_t
  AdvertisedWindowSize(bool scale = true) const override;

  //Override this to always set mptcp enabled to true
  virtual void SetMptcpEnabled (bool flag) override;
  
  //Find net device with given address
  Ptr<NetDevice> MapIpv4ToDevice (Ipv4Address) const;
  Ptr<NetDevice> MapIpv6ToDevice (Ipv6Address) const;
  
  /////////////////////////////////////////////
  //// DSS Mapping handling
  /////////////////////////////////////////////

  /**
   * If no mappings set yet, then it returns the tail ssn of the Tx buffer.
   * Otherwise it returns the last registered mapping TailSequence
   */
  SequenceNumber32 FirstUnmappedSSN();

  /**
   * \brief Creates a DSS option if does not exist and configures it to have a dataack
   * TODO what happens if existing datack already set ?
   */
  virtual void AppendDSSAck();

  /**
   * rename to addDSSFin
   */
  virtual void AppendDSSFin();
  virtual void AppendDSSMapping(Ptr<const MpTcpMapping> mapping);

  virtual void ReceivedAck(Ptr<Packet>, const TcpHeader&); // Received an ACK packet

  virtual void ReceivedData(Ptr<Packet>, const TcpHeader&);
  
  /**

  */
  uint32_t
  SendDataPacket(TcpHeader& header, SequenceNumber32 ssn, uint32_t maxSize);
  
  virtual void
  ReTxTimeout();
  /**
  This one overridesprevious one, adding MPTCP options when needed
  */
  virtual void
  SendEmptyPacket(uint8_t flags); // Send a empty packet that carries a flag, e.g. ACK

  virtual void
  SendEmptyPacket(TcpHeader& header);
  
  virtual Ptr<TcpSocketImpl> Fork (void) override; // Call CopyObject<> to clone me
  
  Ptr<const TcpOptionMpTcpMain> GetMptcpOptionWithSubtype (const TcpHeader& header, TcpOptionMpTcpMain::SubType subtype);


  /* TODO should be able to use parent's one little by little
  */
  virtual void
  CancelAllTimers(void); // Cancel all timer when endpoint is deleted

  uint16_t m_routeId;   //!< Subflow's ID (TODO rename into subflowId ). Position of this subflow in MetaSock's subflows std::vector


  MpTcpMappingContainer m_TxMappings;  //!< List of mappings to send
  MpTcpMappingContainer m_RxMappings;  //!< List of mappings to receive


  Ptr<MpTcpMetaSocket> m_metaSocket;    //!< Meta
  virtual void SendPacket(TcpHeader header, Ptr<Packet> p);

//private:

private:
  // Delayed values to
  uint8_t m_dssFlags;           //!< used to know if AddMpTcpOptions should send a flag
  Ptr<MpTcpMapping> m_dssMapping;    //!< Pending ds configuration to be sent in next packet


  bool m_backupSubflow; //!< Priority
  bool m_masterSocket;  //!< True if this is the first subflow established (with MP_CAPABLE)

  uint32_t m_localNonce;  //!< Store local host token, generated during the 3-way handshake

  uint32_t m_id; //!<Subflow identifier, used for debug purposes

};

}
#endif /* MP_TCP_SUBFLOW */
