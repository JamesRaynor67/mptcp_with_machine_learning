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
#ifndef MPTCP_SOCKET_BASE_H
#define MPTCP_SOCKET_BASE_H

#include "ns3/callback.h"
#include "mptcp-mapping.h"
#include "tcp-socket-impl.h"
#include "tcp-socket-base.h"
//#include "ns3/mp-tcp-path-manager.h"
//#include "ns3/gnuplot.h"
//#include "mp-tcp-subflow.h"

//#include "ns3/mp-tcp-cc.h"
#include "ns3/inet-socket-address.h"
#include "mptcp-scheduler-round-robin.h"
#include "tcp-parameters.h"

//using namespace std;

namespace ns3
{
class Ipv4EndPoint;
class Node;
class Packet;
class TcpL4Protocol;
class MpTcpPathIdManager;
class MpTcpSubflow;
class TcpOptionMpTcpDSS;
class TcpOptionMpTcpJoin;
class OutputStreamWrapper;

using namespace std;

/*
 Packet tag to keep track of subflows ID's
 */
class MpTcpSubflowTag : public Tag {
  
public:
  
  MpTcpSubflowTag();
  ~MpTcpSubflowTag();
  
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;
  
  uint32_t GetSerializedSize (void) const;
  void Serialize (TagBuffer i) const;
  void Deserialize (TagBuffer i);
  void Print (std::ostream &os) const;
  
  uint32_t GetSubflowId() const;
  void SetSubflowId(uint32_t subflowId);
  
  uint32_t GetSourceToken () const;
  void SetSourceToken (uint32_t token);
  
  uint32_t GetDestToken () const;
  void SetDestToken (uint32_t token);
  
private:
  uint32_t m_id;
  uint32_t m_sourceToken;
  uint32_t m_destToken;
};
  
/**
* \class MpTcpMetaSocket

This is the MPTCP meta socket the application talks with
this socket. New subflows, as well as the first one (the master
socket) are linked to this meta socket.

Every data transfer happens on a subflow.
Following the linux kernel from UCL (http://multipath-tcp.org) convention,
the first established subflow is called the "master" subflow.

This inherits from TcpSocketImpl so that it can be  used as any other TCP variant:
this is the backward compability feature that is required in the RFC.
Also doing so allows to run TCP tests with MPTCP via for instance the command
Config::SetDefault ("ns3::TcpL4Protocol::SocketType", "ns3::MpTcpOlia");

As in linux, the meta should return the m_endPoint information of the master,
even if that subflow got closed during the MpTcpConnection.
 
**/
class MpTcpMetaSocket : public TcpSocketImpl

{
public:
  
  static TypeId GetTypeId(void);
  
  virtual TypeId GetInstanceTypeId (void) const override;
  
  MpTcpMetaSocket();
  MpTcpMetaSocket(const MpTcpMetaSocket& sock);
  
  virtual ~MpTcpMetaSocket();
  
  typedef enum {
    MptcpMetaClosed = 0,      /**< Socket is closed  */
    MptcpMetaListening,       /**< Listening for a connection */
    MptcpMetaPreEstablished,  /**< Master socket is in SYN_SENT or SYN_RCVD state */
    MptcpMetaEstablished,     /**< Connection is established */
    MptcpMetaCloseWait,       /**< Remote side has shutdown and is waiting for
                               *  us to finish writing our data and to shutdown
                               *  (we have to close() to move on to LAST_ACK) */
    MptcpMetaLastAck,         /**< Our side has shutdown after remote has
                               *  shutdown.  There may still be data in our
                               *  buffer that we have to finish sending  */
    MptcpMetaFinWait1,   /**< Our side has shutdown, waiting to complete
                          *  transmission of remaining buffered data  */
    MptcpMetaFinWait2,   /**< All buffered data sent, waiting for remote to shutdown */
    MptcpMetaClosing,      /**< Both sides have shutdown but we still have
                            *  data we have to finish sending */
    MptcpMetaTimeWait    /**< Timeout to catch resent junk before entering
                          *  closed, can only be entered from FIN_WAIT2
                          *  or CLOSING.  Required because the other end
                          *  may not have gotten our last ACK causing it
                          *  to retransmit the data packet (which we ignore) */
  } MpTcpMetaSocketState;
  
  /**
   * Called by a sublflow to update the receive window size
   */
  bool UpdateWindowSize (uint32_t windowSize);
  
  /*
   * Whether or not to append a subflow tag to the outgoing packets
   */
  
  bool GetTagSubflows () const;
  void SetTagSubflows (bool value);
  
  /*********************************************
   * Interface methods inherited from Socket
   *********************************************/
  
  virtual int Bind() override;
  virtual int Bind(const Address &address) override;
  virtual int Bind6 () override;
  virtual int Connect(const Address &address) override;
  virtual int Listen(void) override;
  
  /**
   * \brief Same as TcpSocket::Close
   *
   * The default behavior is to do nothing until all the data is transmitted.
   * Only then are
  RFC 6824:
   - When an application calls close() on a socket, this indicates that it
   has no more data to send; for regular TCP, this would result in a FIN
   on the connection.  For MPTCP, an equivalent mechanism is needed, and
   this is referred to as the DATA_FIN.

   - A DATA_FIN has the semantics and behavior as a regular TCP FIN, but
   at the connection level.  Notably, it is only DATA_ACKed once all
   data has been successfully received at the connection level
   */
  
  virtual int Close(void) override;
  virtual int ShutdownSend (void) override;
  virtual int ShutdownRecv (void) override;
  
  virtual uint32_t GetTxAvailable() const override;
  virtual uint32_t GetRxAvailable(void) const override;
  
  virtual SequenceNumber64 GetNextTxSequence() const;
  
  /** Inherit from Socket class: Return data to upper-layer application. Parameter flags
   is not used. Data is returned as a packet of size no larger than maxSize */
  virtual Ptr<Packet> RecvFrom (uint32_t maxSize, uint32_t flags, Address &fromAddress) override;
  virtual Ptr<Packet> Recv(uint32_t maxSize, uint32_t flags) override;
  virtual int Send(Ptr<Packet> p, uint32_t flags) override;
  virtual int SendTo (Ptr<Packet> p, uint32_t flags, const Address &toAddress) override;
  
  //TODO: what does this mean for multipath?
  virtual int GetSockName (Address &address) const override;
  virtual int GetPeerName (Address &address) const override;
  
  virtual enum Socket::SocketErrno GetErrno (void) const override;
  
  virtual Ptr<TcpTxBuffer64> GetTxBuffer (void) const;
  virtual Ptr<TcpRxBuffer64> GetRxBuffer (void) const;
  
  /*****************************************
   * Subflow Management
   ****************************************/
  
  /**
   * return Number of connected subflows (that is that ran the 3whs)
   */
  uint32_t GetNActiveSubflows() const;
  
  
  /**
   * \return an established subflow
   */
  virtual Ptr<MpTcpSubflow> GetActiveSubflow(uint32_t index) const;
  
  virtual Ptr<MpTcpSubflow> GetMaster ();
  
  virtual Ptr<MpTcpSubflow> GetSubflow(uint32_t index);
  
  virtual uint32_t GetNSubflows () const;
  
  /**
   * Called by TcpL4Protocol to create the master subflow
   */
  void CreateMasterSubflow ();
  
  /**
   called by listening subflow when receiving a new SYN with the MP_JOIN option.
   **/
  virtual void NewSubflowJoinRequest(Ptr<MpTcpSubflow> listenSubflow,
                                     Ptr<Packet> p,
                                     const TcpHeader& header,
                                     const Address& fromAddress,
                                     const Address& toAddress,
                                     Ptr<const TcpOptionMpTcpJoin> join);
  
  /********************************************
   * Connection Key and Tokens
   *******************************************/
  
  /**
   @brief According to RFC 6824, the token is used to identify the MPTCP connection and is a
   cryptographic hash of the receiver's key, as exchanged in the initial
   MP_CAPABLE handshake (Section 3.1).  In this specification, the
   tokens presented in this option are generated by the SHA-1 algorithm, truncated to the most significant 32 bits.
   */
  
  uint64_t GetPeerKey() const;
  
  /**
   \note Setting a remote key has the side effect of enabling MPTCP on the socket
   */
  void SetPeerKey(uint64_t aKey);
  
  /**
   \brief Generated during the initial 3 WHS
   */
  uint64_t GetLocalKey() const;
  
  /**
   * \return Hash of the local key
   */
  uint32_t GetLocalToken() const;
  
  /**
   * \return Hash of the peer key
   */
  uint32_t GetPeerToken() const;
  
  /**************************************
   * API for modifying connection level Tx and Rx buffers
   **************************************/
  
  /**
   *  Update tx buffer when we've received a new ack on one of the subflows
   * @brief Called from subflows when they receive DATA-ACK.
   */
  virtual void NewAck(Ptr<MpTcpSubflow> subflow, const SequenceNumber64& dataLevelSeq);
  
  // MPTCP specfic version
  virtual void ReceivedAck (Ptr<MpTcpSubflow> sf, const SequenceNumber64& dack);
  
  /**
   We need to update the connection level receive buffer on receiving new data,
   and notify the app.
   */
  virtual void OnSubflowRecv(Ptr<MpTcpSubflow> sf,
                             Ptr<Packet> p,
                             const TcpHeader& tcpHeader,
                             SequenceNumber64 expectedDSN,
                             Ptr<MpTcpMapping> mapping);
  
  virtual bool AddToReceiveBuffer(Ptr<MpTcpSubflow> sf,
                                  Ptr<Packet> p,
                                  const TcpHeader& tcpHeader,
                                  Ptr<MpTcpMapping> mapping);
  
  /***************************************
   * Subflow Callbacks
   ***************************************/
  /**
   these callbacks will be passed on to
   * \see Socket::Set
   */
  virtual void SetSubflowAcceptCallback(Callback<bool, Ptr<MpTcpMetaSocket>, const Address&, const Address&> connectionRequest,
                                        Callback<void, Ptr<MpTcpSubflow>, const Address&> connectionCreated);
  
  virtual void SetSubflowConnectCallback(Callback<void, Ptr<MpTcpSubflow> > connectionSucceeded,
                                         Callback<void, Ptr<MpTcpSubflow> > connectionFailure);
  
  virtual void SetSubflowAddedCallback(Callback<void, Ptr<MpTcpSubflow>, bool> subflowAdded);
  
  static void
  NotifySubflowUpdateCwnd(Ptr<MpTcpMetaSocket> meta,
                          Ptr<MpTcpSubflow> sf,
                          uint32_t oldCwnd,
                          uint32_t newCwnd);
  
  /**
   * Called when a subflow TCP state is updated.
   * It detects such events by tracing its subflow m_state.
   *
   */
  virtual void OnSubflowUpdateCwnd(Ptr<MpTcpSubflow> subflow, uint32_t oldCwnd, uint32_t newCwnd);
  
  
  static void
  NotifySubflowNewState(Ptr<MpTcpMetaSocket> meta,
                        Ptr<MpTcpSubflow> sf,
                        TcpSocket::TcpStates_t  oldState,
                        TcpSocket::TcpStates_t newState);
  
  /**
   * Called when a subflow TCP state is updated.
   * It detects such events by tracing its subflow m_state.
   *
   * \param sf Subflow that changed state
   * \param oldState previous TCP state of the subflow
   * \param newState new TCP state of the subflow
   */
  virtual void OnSubflowNewState(Ptr<MpTcpSubflow> sf,
                                 TcpStates_t oldState,
                                 TcpStates_t newState);
  
  
  /**
   * Initiates a new subflow with MP_JOIN
   *
   * Wrapper that just creates a subflow, bind it to a specific address
   * and then establishes the connection
   */
  virtual int ConnectNewSubflow(const Address &local, const Address &remote);
  
  
  
  
  /*********************************************
   * Connection Establishment
   *********************************************/
  
  /**
   * The connection is considered fully established
   * when it can create new subflows, i.e., when it received
   * a first dss ack
   */
  virtual bool FullyEstablished() const;

  /**
   * This retriggers Connection success callback
   * You have to check in the callback if it fully estalbished or not
   */
  virtual void BecomeFullyEstablished();
  
  virtual void SetFullyEstablishedCallback (Callback<void, Ptr<MpTcpMetaSocket>> callback);

  bool IsConnected() const;
  
  virtual void EstablishSubflow(Ptr<MpTcpSubflow> subflow, Ptr<Packet> packet, const TcpHeader& tcpHeader);
  
  
  virtual void DoRetransmit();
  
  // Window Management
  
  void DumpRxBuffers(Ptr<MpTcpSubflow> sf) const;
  
  virtual uint32_t AvailableWindow();
  virtual uint32_t GetRwndSize ();
  virtual uint32_t UnAckDataCount ();
  virtual uint32_t GetTotalCwnd ();
  
  virtual void PersistTimeout();
  
  
  virtual void OnInfiniteMapping(Ptr<TcpOptionMpTcpDSS> dss, Ptr<MpTcpSubflow> sf);
  
  virtual void Destroy(void);
  
  /**
   For now it looks there is no way to know that an ip interface went up so we will assume until
   further notice that IPs of the client don't change.
   -1st callback called on receiving an ADD_ADDR
   -2nd callback called on receiving REM_ADDR
   (TODO this class should automatically register)
   **/
  void SetNewAddrCallback(Callback<bool, Ptr<Socket>, Address, uint8_t> remoteAddAddrCb,
                          Callback<void, uint8_t> remoteRemAddrCb);

  /**
   *
   */
  void GetAllAdvertisedDestinations(vector<InetSocketAddress>& );
  
  
  virtual void CompleteFork(Ptr<Packet> p, const TcpHeader& h,
                            const Address& fromAddress, const Address& toAddress) override;

protected: // protected methods
  
  friend class MpTcpSubflow;
  
  /*********************************************
   * Connection Termination
   *********************************************/
  
  /**
   RFC 6824
   - If all subflows have
   been closed with a FIN exchange, but no DATA_FIN has been received
   and acknowledged, the MPTCP connection is treated as closed only
   after a timeout.  This implies that an implementation will have
   TIME_WAIT states at both the subflow and connection levels (see
   Appendix C).  This permits "break-before-make" scenarios where
   connectivity is lost on all subflows before a new one can be re-
   established.
   */
  
  
  /* equivalent to PeerClose
   \param finalDsn
   OnDataFin
   */
  virtual void PeerClose(const SequenceNumber64& dsn, const SequenceNumber64& dack, Ptr<MpTcpSubflow> sf);
  
  /* called by subflow when it sees a DSS with the DATAFIN flag
   // DATA FIN is in sequence, notify app and respond with a Data ACK
   */
  
  virtual void DoPeerClose(Ptr<MpTcpSubflow> sf);
  
  virtual int DoClose(void);
  
  virtual void CloseAndNotify(void); //Nofity application
  
  virtual void LastAckTimeout();
  
  
  /* equivalent to TCP Rst */
  virtual void SendFastClose(Ptr<MpTcpSubflow> sf);
  
  /** 
   * Sends a single MP_FIN
   */
  virtual void SendDataFin(bool withAck);
  
  //Sends a single DATA_ACK on given subflow
  virtual void SendDataAck (Ptr<MpTcpSubflow> sf);

  virtual void CancelAllEvents ();
  
  //Called from a subflow to append a DATA_FIN to a data packet if there is no remaining data, and
  //close on empty is enabled
  //Changes the state
  virtual bool CheckAndAppendDataFin (Ptr<MpTcpSubflow> subflow, SequenceNumber32 ssn,
                                      uint32_t length, Ptr<MpTcpMapping> mapping);
  
  /**
   * Should be called after having receiving a Data ACK in response to a sent DataFIN
   * Should send a RST on all subflows in state Other
   * and a FIN for Established subflows
   */
  virtual void CloseAllSubflows();
  
  // MPTCP connection and subflow set up
  
  /* close all subflows
   */
  virtual void TimeWait();
  
  virtual void OnTimeWaitTimeOut();
  
  /*********************************
   * Subflow Creation
   ********************************/
  
  /**
   * \brief
   * \param masterSocket Whether the new subflow is a master subflow
   */
  Ptr<MpTcpSubflow> CreateSubflow(bool masterSocket);
  
  /**
   * Add subflow to the subflows list, and register trace functions
   */
  virtual void AddSubflow(Ptr<MpTcpSubflow> sf,  bool isMaster);
  
  /*
   Remove subflow from containers
   TODO should also erase its id from the path id manager
   \sf
   \param reset True if closing due to reset
   */
  void OnSubflowClosed(Ptr<MpTcpSubflow> sf, bool reset);
  
  void OnSubflowDupAck(Ptr<MpTcpSubflow> sf);
  
  /**
   Called when a subflow that initiated the connection
   gets established
   
   TODO rename into ConnectionSucceeded
   Notify ?
   **/
  virtual void OnSubflowEstablishment(Ptr<MpTcpSubflow>);
  
  /**
   Called when a subflow that received a connection
   request gets established
   
   TODO I don't like the name,rename later
   */
  virtual void OnSubflowEstablished(Ptr<MpTcpSubflow> subflow);
  
  /**
   Should be called when subflows enters FIN_WAIT or LAST_ACK
   */
  virtual void OnSubflowClosing(Ptr<MpTcpSubflow>);
  
  //When a subflow calls NofityConnectionRequest
  //virtual void OnSubflowConnectionRequest(const Address &from);
  
  
  
  //////////////////////////////////////////////////////////////////
  ////  Here follows a list of MPTCP specific *callbacks* triggered by subflows
  ////  on certain events
  
  /**
   * @param
   * @param mapping
   add count param ?
   */
  virtual void OnSubflowDupack(Ptr<MpTcpSubflow> sf, MpTcpMapping mapping);
  
  virtual void OnSubflowRetransmit(Ptr<MpTcpSubflow> sf);
  
  /****************************
   * Notify upper layer callbacks
   ****************************/
  
  virtual void NotifyFullyEstablished ();
  
  /**
   * Triggers when a listening meta socket receives a new SYN with an MP_JOIN option.
   */
  virtual bool NotifyJoinRequest (const Address &from, const Address & toAddress);
  
  /**
   * Callbacks for the Connect() subflow socket call
   * Triggers callback registered by SetSubflowConnectCallback
   */
  virtual void NotifySubflowConnectionFailure (Ptr<Socket> socket);
  virtual void NotifySubflowConnectionSuccess (Ptr<Socket> socket);

  //Callbacks for the Accept() subflow socket call
  virtual bool NotifySubflowConnectionRequest(Ptr<Socket> socket, const Address &from);
  virtual void NotifySubflowNewConnectionCreated(Ptr<Socket> socket, const Address &from);

  //When a subflow calls NotifyDataSent
  virtual void NotifySubflowDataSent(Ptr<Socket> socket, uint32_t dataSent);
  
  /**
   * \brief Call CopyObject<> to clone me
   * \returns a copy of the socket
   */
  virtual Ptr<TcpSocketImpl> Fork (void) override;
  
  virtual Ptr<TcpSocketImpl> Fork (Ptr<MpTcpSubflow> subflow);

  /**
   * Expects Ipv4 (6 not supported yet)
   */
  bool OwnIP(const Address& address) const;
  
  
  /**
   Fails if
   **/
  //  bool AddLocalAddress(uint8_t&, Port);
  // Should generate an Id
  
  //  void SetAddrEventCallback(Callback<bool, Ptr<Socket>, Address, uint8_t> remoteAddAddrCb,
  //                          Callback<void, uint8_t> remoteRemAddrCb);
  //virtual RemoteAddAddr
  void NotifyRemoteAddAddr(Address address);
  void NotifyRemoteRemAddr(uint8_t addrId);
  
  
  virtual void ConnectionSucceeded(Ptr<MpTcpSubflow> subflow);
  
  /**
   * Sending data via subflows with available window size. It sends data only to ESTABLISHED subflows.
   * It sends data by calling SendDataPacket() function.
   * This one is really different from parent class

   *
   * Called by functions: ReceveidAck, NewAck
   * send as  much as possible
   * \return true if it send mappings
   */
  virtual bool SendPendingData();
  
  
  virtual void ReTxTimeout();


  virtual void Retransmit();
  
  
  /** Does nothing */
  //  virtual void EstimateRtt (const TcpHeader&);
  
  
  //  Time ComputeReTxTimeoutForSubflow( Ptr<MpTcpSubflow> );
  
  /**
   * Part of the logic was implemented but this is non-working.
   * \return Always false
   */
  virtual bool IsInfiniteMappingEnabled() const;
  virtual bool DoChecksum() const;
  
  /**
   * TODO should accept a stream
   *
   */
  virtual void DumpSubflows() const;

  
  //  Callback<void, const MpTcpAddressInfo& > m_onRemAddr;
  
  //  virtual void OnAddAddress(MpTcpAddressInfo);
  //  virtual void OnRemAddress();
  
  virtual void CreateScheduler(TypeId schedulerTypeId);
  
  /**
   * Generate a unique key for this host
   *
   * \see mptcp_set_key_sk
   */
  virtual uint64_t GenerateUniqueMpTcpKey();
  
  typedef vector<Ptr<MpTcpSubflow>> SubflowList;
  SubflowList GetSubflowsWithState(TcpStates_t state);
  
  Callback<void, Ptr<MpTcpSubflow> > m_subflowConnectionSucceeded;  //!< connection succeeded callback
  Callback<void, Ptr<MpTcpSubflow> > m_subflowConnectionFailure;    //!< connection failed callback
  Callback<void, Ptr<MpTcpSubflow>, const Address&> m_subflowConnectionCreated; //!< subflow created as a result of SYN request
  Callback<bool, Ptr<MpTcpMetaSocket>, const Address&, const Address&> m_joinRequest;    //!< connection request callback
  Callback<void, Ptr<MpTcpSubflow>, bool> m_subflowAdded; //!<subflow added to the meta socket, after it is created
  //  Callback<void, Ptr<Socket> >                   m_normalClose;          //!< connection closed callback
  //  Callback<void, Ptr<Socket> >                   m_errorClose;           //!< connection closed due to errors callback
  
  
  Callback<bool, Ptr<Socket>, Address, uint8_t> m_onRemoteAddAddr;  //!< return true to create a subflow
  //  Callback<bool, Ptr<Socket>, Address, uint8_t > m_onNewLocalIp;  //!< return true to create a subflow
  Callback<void, uint8_t> m_onAddrDeletion;    // return true to create a subflow
  
  Callback<void, Ptr<MpTcpMetaSocket>>  m_connectionFullyEstablished;  //!< connection fully established callback
  
  // , const Address &, bool master
  //  Callback<void, Ptr<MpTcpSubflow> >    m_subflowConnectionSucceeded; //!< connection created callback
  
  // TODO rename since will track local too.
  Ptr<MpTcpPathIdManager> m_remotePathIdManager;  //!< Keep track of advertised ADDR id advertised by remote endhost
  
  SubflowList m_subflows;
  SubflowList m_activeSubflows; //Keep track of all the established subflows
  
  Ptr<MpTcpSubflow> m_master;
  
  // Rx and Tx buffer management
  Ptr<TcpRxBuffer64>        m_rxBuffer;       //!< Rx buffer (reordering buffer)
  Ptr<TcpTxBuffer64>        m_txBuffer;       //!< Tx buffer
  
  
  Ptr<MpTcpScheduler> m_scheduler;  //!<
  
  MpTcpMetaSocketState m_state;
  
  uint64_t m_localKey;    //!< Store local host token, generated during the 3-way handshake
  uint32_t m_localToken;  //!< Generated from key
  
  uint64_t m_peerKey; //!< Store remote host token
  uint32_t m_peerToken;
  
  bool     m_doChecksum;  //!< Compute the checksum. Negociated during 3WHS. Unused
  bool     m_receivedDSS;  //!< True if we received at least one DSS
  bool     m_connected;
  bool     m_tagSubflows;  //!<Whether or not to add the subflow packet tag
  
  //!
  TypeId m_subflowTypeId;
  TypeId m_schedulerTypeId;
  
  TracedValue<uint32_t>   m_rWnd;        //!< Receiver window (RCV.WND in RFC793)
  uint32_t                m_initialCWnd;     //!< Initial cWnd value
  uint32_t                m_initialSsThresh; //!< Initial Slow Start Threshold value
  uint32_t                m_segmentSize;     //!< Segment size
  SequenceNumber32        m_lastAckedSeq;    //!< Last sequence ACKed
  TracedValue<SequenceNumber64> m_highTxMark; //!< Highest seqno ever sent, regardless of ReTx
  TracedValue<SequenceNumber64> m_nextTxSequence; //!< Next seqnum to be sent (SND.NXT), ReTx pushes it back
  mutable enum SocketErrno m_errno;         //!< Socket error code
  EventId m_sendPendingDataEvent; //!< micro-delay event to send pending data
  EventId           m_retxEvent;       //!< Retransmission event
  EventId           m_lastAckEvent;
  EventId           m_timeWaitEvent;
  
  //Inherited from TcpSocket, setting the TCP parameters
  
  virtual void NotifyRcvBufferChange (uint32_t oldSize, uint32_t newSize) override;
  
  virtual void SetSndBufSize (uint32_t size) override;
  virtual uint32_t GetSndBufSize (void) const override;
  virtual void SetRcvBufSize (uint32_t size) override;
  virtual uint32_t GetRcvBufSize (void) const override;
  virtual void SetSegSize (uint32_t size) override;
  virtual uint32_t GetSegSize (void) const override;
  virtual void SetInitialSSThresh (uint32_t threshold) override;
  virtual uint32_t GetInitialSSThresh (void) const override;
  virtual void SetInitialCwnd (uint32_t cwnd) override;
  virtual uint32_t GetInitialCwnd (void) const override;
  
  virtual void SetMptcpEnabled (bool flag) override;
  
};
  
}   //namespace ns3

#endif /* MP_TCP_SOCKET_BASE_H */
