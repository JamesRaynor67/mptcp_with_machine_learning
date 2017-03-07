/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 Georgia Tech Research Corporation
 * Copyright (c) 2010 Adrian Sai-wah Tam
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
 * Author: Adrian Sai-wah Tam <adrian.sw.tam@gmail.com>
 */
#ifndef TCP_SOCKET_BASE_H
#define TCP_SOCKET_BASE_H

#include <stdint.h>
#include <queue>
#include "ns3/callback.h"
#include "ns3/traced-value.h"
#include "tcp-socket-impl.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv6-header.h"
#include "ns3/ipv6-interface.h"
#include "ns3/event-id.h"
#include "tcp-tx-buffer.h"
#include "tcp-rx-buffer.h"
#include "rtt-estimator.h"
#include "tcp-parameters.h"
#include "tcp-socket-state.h"

namespace ns3 {


#define DISABLE_MEMBER(retType,member) retType \
              MpTcpSubflow::member(void) {\
              NS_FATAL_ERROR("This should never be called. The meta will make the subflow pass from LISTEN to ESTABLISHED."); \
              }


class Ipv4EndPoint;
class Ipv6EndPoint;
class Node;
class Packet;
class TcpL4Protocol;
class TcpHeader;
class MpTcpSubflow;
class MpTcpMetaSocket;
class TcpTraceHelper;
class TcpCongestionOps;

/**
 * \ingroup tcp
 *
 * \brief Helper class to store RTT measurements
 */
class RttHistory {
public:
  /**
   * \brief Constructor - builds an RttHistory with the given parameters
   * \param s First sequence number in packet sent
   * \param c Number of bytes sent
   * \param t Time this one was sent
   */
  RttHistory (SequenceNumber32 s, uint32_t c, Time t);
  /**
   * \brief Copy constructor
   * \param h the object to copy
   */
  RttHistory (const RttHistory& h); // Copy constructor
public:
  SequenceNumber32  seq;  //!< First sequence number in packet sent
  uint32_t        count;  //!< Number of bytes sent
  Time            time;   //!< Time this one was sent
  bool            retx;   //!< True if this has been retransmitted
};

/// Container for RttHistory objects
typedef std::deque<RttHistory> RttHistory_t;

/**
 * \ingroup socket
 * \ingroup tcp
 *
 * \brief A base class for implementation of a stream socket using TCP.
 *
 * This class contains the essential components of TCP, as well as a sockets
 * interface for upper layers to call. This serves as a base for other TCP
 * functions where the sliding window mechanism is handled here. This class
 * provides connection orientation and sliding window flow control. Part of
 * this class is modified from the original NS-3 TCP socket implementation
 * (TcpSocketImpl) by Raj Bhattacharjea <raj.b@gatech.edu> of Georgia Tech.
 *
 * For IPv4 packets, the TOS set for the socket is used. The Bind and Connect
 * operations set the TOS for the socket to the value specified in the provided
 * address. A SocketIpTos tag is only added to the packet if the resulting
 * TOS is non-null.
 * Each packet is assigned the priority set for the socket. Setting a TOS
 * for a socket also sets a priority for the socket (according to the
 * Socket::IpTos2Priority function). A SocketPriority tag is only added to the
 * packet if the priority is non-null.
 *
 * Congestion state machine
 * ---------------------------
 *
 * The socket maintains two state machines; the TCP one, and another called
 * "Congestion state machine", which keeps track of the phase we are in. Currently,
 * ns-3 manages the states:
 *
 * - CA_OPEN
 * - CA_DISORDER
 * - CA_RECOVERY
 * - CA_LOSS
 *
 * Another one (CA_CWR) is present but not used. For more information, see
 * the TcpCongState_t documentation.
 *
 * Congestion control interface
 * ---------------------------
 *
 * Congestion control, unlike older releases of ns-3, has been splitted from
 * TcpSocketBase. In particular, each congestion control is now a subclass of
 * the main TcpCongestionOps class. Switching between congestion algorithm is
 * now a matter of setting a pointer into the TcpSocketBase class.
 *
 * The variables needed to congestion control classes to operate correctly have
 * been moved inside the TcpSocketState class. It contains information on the
 * congestion window, slow start threshold, segment size and the state of the
 * Congestion state machine.
 *
 * To track the trace inside the TcpSocketState class, a "forward" technique is
 * used, which consists in chaining callbacks from TcpSocketState to TcpSocketBase
 * (see for example cWnd trace source).
 *
 * Fast retransmit
 * ---------------------------
 *
 * The fast retransmit enhancement is introduced in RFC 2581 and updated in
 * RFC 5681. It basically reduces the time a sender waits before retransmitting
 * a lost segment, through the assumption that if it receives a certain number
 * of duplicate ACKs, a segment has been lost and it can be retransmitted.
 * Usually it is coupled with the Limited Transmit algorithm, defined in
 * RFC 3042.
 *
 * In ns-3, these algorithms are included in this class, and it is implemented inside
 * the ReceivedAck method. The attribute which manages the number of dup ACKs
 * necessary to start the fast retransmit algorithm is named "ReTxThreshold",
 * and its default value is 3, while the Limited Transmit one can be enabled
 * by setting the attribute "LimitedTransmit" to true.
 *
 * Fast recovery
 * --------------------------
 *
 * The fast recovery algorithm is introduced RFC 2001, and it basically
 * avoids to reset cWnd to 1 segment after sensing a loss on the channel. Instead,
 * the slow start threshold is halved, and the cWnd is set equal to such value,
 * plus segments for the cWnd inflation.
 *
 * The algorithm is implemented in the ReceivedAck method.
 *
 */
class TcpSocketBase : public TcpSocketImpl
{
public:
  /**
   * Get the type ID.
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  
  /**
   * \brief Get the instance TypeId
   * \return the instance TypeId
   */
  virtual TypeId GetInstanceTypeId () const;

  friend class TcpGeneralTest;

  /**
   * Create an unbound TCP socket
   */
  TcpSocketBase (void);

  /**
   * Clone a TCP socket, for use upon receiving a connection request in LISTEN state
   *
   * \param sock the original Tcp Socket
   */
  TcpSocketBase (const TcpSocketBase& sock);
  virtual ~TcpSocketBase (void);
  
  /**
   * \brief Set the first Tx byte not acknowledged yet
   * \param seq First byte unacknowledged
   */
  //  virtual void SetTxHead(const SequenceNumber32& seq);
  
  virtual TcpSocket::TcpStates_t GetState() const;
  
  /**
   * \brief Callback pointer for cWnd trace chaining
   */
  TracedCallback<uint32_t, uint32_t> m_cWndTrace;

  /**
   * \brief Callback pointer for ssTh trace chaining
   */
  TracedCallback<uint32_t, uint32_t> m_ssThTrace;

  /**
   * \brief Callback pointer for congestion state trace chaining
   */
  TracedCallback<TcpSocketState::TcpCongState_t, TcpSocketState::TcpCongState_t> m_congStateTrace;

  /**
   * \brief Callback pointer for high tx mark chaining
   */
  TracedCallback <SequenceNumber32, SequenceNumber32> m_highTxMarkTrace;

  /**
   * \brief Callback pointer for next tx sequence chaining
   */
  TracedCallback<SequenceNumber32, SequenceNumber32> m_nextTxSequenceTrace;

  /**
   * \brief Callback function to hook to TcpSocketState congestion window
   * \param oldValue old cWnd value
   * \param newValue new cWnd value
   */
  void UpdateCwnd (uint32_t oldValue, uint32_t newValue);

  /**
   * \brief Callback function to hook to TcpSocketState slow start threshold
   * \param oldValue old ssTh value
   * \param newValue new ssTh value
   */
  void UpdateSsThresh (uint32_t oldValue, uint32_t newValue);

  /**
   * \brief Callback function to hook to TcpSocketState congestion state
   * \param oldValue old congestion state value
   * \param newValue new congestion state value
   */
  void UpdateCongState (TcpSocketState::TcpCongState_t oldValue,
                        TcpSocketState::TcpCongState_t newValue);

  /**
   * \brief Callback function to hook to TcpSocketState high tx mark
   * \param oldValue old high tx mark
   * \param newValue new high tx mark
   */
  void UpdateHighTxMark (SequenceNumber32 oldValue, SequenceNumber32 newValue);
  
  /**
   * \brief Callback function to hook to TcpSocketState next tx sequence
   * \param oldValue old nextTxSeq value
   * \param newValue new nextTxSeq value
   */
  void UpdateNextTxSequence (SequenceNumber32 oldValue, SequenceNumber32 newValue);
  
  // HACK MATT
  virtual void GenerateEmptyPacketHeader(TcpHeader& header, uint8_t flags);
  
  // Necessary implementations of null functions from ns3::Socket
  virtual enum SocketErrno GetErrno (void) const override;    // returns m_errno
  virtual int Bind (void) override;    // Bind a socket by setting up endpoint in TcpL4Protocol
  virtual int Bind6 (void) override;    // Bind a socket by setting up endpoint in TcpL4Protocol
  virtual int Bind (const Address &address) override;         // ... endpoint of specific addr or port
  virtual int Connect (const Address &address) override;      // Setup endpoint and call ProcessAction() to connect
  virtual int Listen (void) override;  // Verify the socket is in a correct state and call ProcessAction() to listen
  virtual int Close (void) override;   // Close by app: Kill socket upon tx buffer emptied
  virtual int ShutdownSend (void) override;    // Assert the m_shutdownSend flag to prevent send to network
  virtual int ShutdownRecv (void) override;    // Assert the m_shutdownRecv flag to prevent forward to app
  virtual int Send (Ptr<Packet> p, uint32_t flags) override;  // Call by app to send data to network
  virtual int SendTo (Ptr<Packet> p, uint32_t flags, const Address &toAddress) override; // Same as Send(), toAddress is insignificant
  virtual Ptr<Packet> Recv (uint32_t maxSize, uint32_t flags) override; // Return a packet to be forwarded to app
  virtual Ptr<Packet> RecvFrom (uint32_t maxSize, uint32_t flags, Address &fromAddress) override; // ... and write the remote address at fromAddress
  virtual uint32_t GetTxAvailable (void) const; // Available Tx buffer size
  virtual uint32_t GetRxAvailable (void) const; // Available-to-read data size, i.e. value of m_rxAvailable
  virtual int GetSockName (Address &address) const override; // Return local addr:port in address
  virtual int GetPeerName (Address &address) const override;
  virtual void BindToNetDevice (Ptr<NetDevice> netdevice) override; // NetDevice with my m_endPoint
  
  virtual Ipv4EndPoint* GetEndpoint () const;
  virtual Ipv6EndPoint* GetEndpoint6 () const;
  
  Ptr<TcpTxBuffer32> GetTxBuffer (void) const;
  Ptr<TcpRxBuffer32> GetRxBuffer (void) const;
  
  /**
   * \brief Return unfilled portion of window
   * \return unfilled portion of window
   */
  virtual uint32_t AvailableWindow (void) const;
  
  // Return Peer ISN
  /*virtual SequenceNumber32 GetLocalIsn (void) const;
   virtual SequenceNumber32 GetPeerIsn (void) const;
   
   virtual void InitPeerISN (const SequenceNumber32& seq);
   virtual void InitLocalISN ();
   virtual void InitLocalISN (const SequenceNumber32& seq);*/
  
  /**
   * TracedCallback signature for tcp packet transmission or reception events.
   *
   * \param [in] packet The packet.
   * \param [in] ipv4
   * \param [in] interface
   */
  typedef void (* TcpTxRxTracedCallback) (const Ptr<const Packet> packet, const TcpHeader& header,
  const Ptr<const TcpSocketBase> socket);
  
  /**
   * \brief Check to see if we can send data in the Tx window.
   *
   *
   * \returns true if we can send data
   */
  virtual bool CanSendPendingData (uint32_t dataToSend);
  
  /**
   * \brief Complete a connection by forking the socket
   *
   * This function is called only if a SYN received in LISTEN state. After
   * TcpSocketBase cloned, allocate a new end point to handle the incoming
   * connection and send a SYN+ACK to complete the handshake.
   *
   * \param p the packet triggering the fork
   * \param tcpHeader the TCP header of the triggering packet
   * \param fromAddress the address of the remote host
   * \param toAddress the address the connection is directed to
   */
  virtual void CompleteFork (Ptr<Packet> p, const TcpHeader& tcpHeader,
                             const Address& fromAddress, const Address& toAddress) override;
  
protected:
  
  // Helper functions: Connection set up

  /**
   * \brief Common part of the two Bind(), i.e. set callback and remembering local addr:port
   *
   * \returns 0 on success, -1 on failure
   */
  int SetupCallback (void);

  /**
   * \brief Perform the real connection tasks: Send SYN if allowed, RST if invalid
   *
   * \returns 0 on success
   */
  int DoConnect (void);

  /**
   * \brief Schedule-friendly wrapper for Socket::NotifyConnectionSucceeded()
   */
  virtual void ConnectionSucceeded (void);

  /**
   * \brief Configure the endpoint to a local address. Called by Connect() if Bind() didn't specify one.
   *
   * \returns 0 on success
   */
  int SetupEndpoint (void);

  /**
   * \brief Configure the endpoint v6 to a local address. Called by Connect() if Bind() didn't specify one.
   *
   * \returns 0 on success
   */
  int SetupEndpoint6 (void);

  
  
  // Helper functions: Transfer operation

  /**
   * \brief Called by the L3 protocol when it received a packet to pass on to TCP.
   *
   * \param packet the incoming packet
   * \param header the packet's IPv4 header
   * \param port the remote port
   * \param incomingInterface the incoming interface
   */
  virtual void ForwardUp (Ptr<Packet> packet, Ipv4Header header, uint16_t port, Ptr<Ipv4Interface> incomingInterface);

  /**
   * \brief Called by the L3 protocol when it received a packet to pass on to TCP.
   *
   * \param packet the incoming packet
   * \param header the packet's IPv6 header
   * \param port the remote port
   * \param incomingInterface the incoming interface
   */
  void ForwardUp6 (Ptr<Packet> packet, Ipv6Header header, uint16_t port, Ptr<Ipv6Interface> incomingInterface);

  /**
   * \brief Called by TcpSocketBase::ForwardUp{,6}().
   *
   * Get a packet from L3. This is the real function to handle the
   * incoming packet from lower layers. This is
   * wrapped by ForwardUp() so that this function can be overloaded by daughter
   * classes.
   *
   * \param packet the incoming packet
   * \param fromAddress the address of the sender of packet
   * \param toAddress the address of the receiver of packet (hopefully, us)
   */
  virtual void DoForwardUp (Ptr<Packet> packet, const Address &fromAddress,
                            const Address &toAddress);

  /**
   * \brief Called by the L3 protocol when it received an ICMP packet to pass on to TCP.
   *
   * \param icmpSource the ICMP source address
   * \param icmpTtl the ICMP Time to Live
   * \param icmpType the ICMP Type
   * \param icmpCode the ICMP Code
   * \param icmpInfo the ICMP Info
   */
  void ForwardIcmp (Ipv4Address icmpSource, uint8_t icmpTtl, uint8_t icmpType, uint8_t icmpCode, uint32_t icmpInfo);

  /**
   * \brief Called by the L3 protocol when it received an ICMPv6 packet to pass on to TCP.
   *
   * \param icmpSource the ICMP source address
   * \param icmpTtl the ICMP Time to Live
   * \param icmpType the ICMP Type
   * \param icmpCode the ICMP Code
   * \param icmpInfo the ICMP Info
   */
  void ForwardIcmp6 (Ipv6Address icmpSource, uint8_t icmpTtl, uint8_t icmpType, uint8_t icmpCode, uint32_t icmpInfo);

  /**
   * \brief Send as much pending data as possible according to the Tx window.
   *
   *
   * Note that this function did not implement the PSH flag.
   *
   * \param withAck forces an ACK to be sent
   * \returns true if some data have been sent
   */
  virtual bool SendPendingData (bool withAck = false);

  /**
   * \Brief Will generate the header and forward it to its overload.
   *
   * \see SendDataPacket
   */
  virtual uint32_t SendDataPacket (SequenceNumber32 seq, uint32_t maxSize, bool withAck);

  /**
   * \brief Extract at most maxSize bytes from the TxBuffer at sequence seq, add the
   *        TCP header, and send to TcpL4Protocol
   *
   * \param seq the sequence number
   * \param maxSize the maximum data block to be transmitted (in bytes)
   * \param withAck forces an ACK to be sent
   * \returns the number of bytes sent
   */
  virtual uint32_t SendDataPacket (TcpHeader& header, SequenceNumber32 seq, uint32_t maxSize);

  /**
   * \brief Generates the header and calls its overload
   *
   * \see SendEmptyPacket
   */
  virtual void SendEmptyPacket (uint8_t flags);

  /**
   * \brief Send a empty packet that carries a flag, e.g. ACK
   *
   * \param flags the packet's flags
   */
  virtual void SendEmptyPacket (TcpHeader& header);

  /**
   * \brief
   *
   * \param header A valid TCP header
   * \param p Packet to send. May be empty.
   */
  virtual void SendPacket(TcpHeader header, Ptr<Packet> p);

  /**
   * \brief Send reset and tear down this socket
   */
  virtual void SendRST (void);

  /**
   * \brief Check if a sequence number range is within the rx window
   *
   * \param head start of the Sequence window
   * \param tail end of the Sequence window
   * \returns true if it is in range
   */
  virtual bool OutOfRange (SequenceNumber32 head, SequenceNumber32 tail) const;


  // Helper functions: Connection close

  /**
   * \brief Close a socket by sending RST, FIN, or FIN+ACK, depend on the current state
   *
   * \returns 0 on success
   */
  virtual int DoClose (void);

  /**
   * \brief Peacefully close the socket by notifying the upper layer and deallocate end point
   */
  virtual void CloseAndNotify (void);

  /**
   * \brief Kill this socket by zeroing its attributes (IPv4)
   *
   * This is a callback function configured to m_endpoint in
   * SetupCallback(), invoked when the endpoint is destroyed.
   */
  virtual void Destroy (void);

  /**
   * \brief Kill this socket by zeroing its attributes (IPv6)
   *
   * This is a callback function configured to m_endpoint in
   * SetupCallback(), invoked when the endpoint is destroyed.
   */
  virtual void Destroy6 (void);

  /**
   * \brief Deallocate m_endPoint and m_endPoint6
   */
  virtual void DeallocateEndPoint (void);

  /**
   * \brief Received a FIN from peer, notify rx buffer
   *
   * \param p the packet
   * \param tcpHeader the packet's TCP header
   */
  virtual void PeerClose (Ptr<Packet> p, const TcpHeader& tcpHeader);

  /**
   * \brief FIN is in sequence, notify app and respond with a FIN
   */
  virtual void DoPeerClose (void);

  /**
   * \brief Cancel all timer when endpoint is deleted
   */
  virtual void CancelAllTimers (void);

  /**
   * \brief Move from CLOSING or FIN_WAIT_2 to TIME_WAIT state
   */
  virtual void TimeWait (void);

  // State transition functions

  /**
   * \brief Received a packet upon ESTABLISHED state.
   *
   * This function is mimicking the role of tcp_rcv_established() in tcp_input.c in Linux kernel.
   *
   * \param packet the packet
   * \param tcpHeader the packet's TCP header
   */
  virtual void ProcessEstablished (Ptr<Packet> packet, const TcpHeader& tcpHeader); // Received a packet upon ESTABLISHED state

  /**
   * \brief Received a packet upon LISTEN state.
   *
   * \param packet the packet
   * \param tcpHeader the packet's TCP header
   * \param fromAddress the source address
   * \param toAddress the destination address
   */
  virtual void ProcessListen (Ptr<Packet> packet, const TcpHeader& tcpHeader,
                      const Address& fromAddress, const Address& toAddress);

  /**
   * \brief Received a packet upon SYN_SENT
   *
   * \param packet the packet
   * \param tcpHeader the packet's TCP header
   */
  virtual void ProcessSynSent (Ptr<Packet> packet, const TcpHeader& tcpHeader);
  
  virtual void EstablishConnection(Ptr<Packet> packet, const TcpHeader& tcpHeader, bool withAck);

  /**
   * \brief Received a packet upon SYN_RCVD.
   *
   * \param packet the packet
   * \param tcpHeader the packet's TCP header
   * \param fromAddress the source address
   * \param toAddress the destination address
   */
  virtual void ProcessSynRcvd (Ptr<Packet> packet, const TcpHeader& tcpHeader,
                       const Address& fromAddress, const Address& toAddress);

  /**
   * \brief Received a packet upon CLOSE_WAIT, FIN_WAIT_1, FIN_WAIT_2
   *
   * \param packet the packet
   * \param tcpHeader the packet's TCP header
   */
  virtual void ProcessWait (Ptr<Packet> packet, const TcpHeader& tcpHeader);
  
  /**
   * \brief Received a packet upon CLOSING
   *
   * \param packet the packet
   * \param tcpHeader the packet's TCP header
   */
  virtual void ProcessClosing (Ptr<Packet> packet, const TcpHeader& tcpHeader);

  /**
   * \brief Received a packet upon LAST_ACK
   *
   * \param packet the packet
   * \param tcpHeader the packet's TCP header
   */
  virtual void ProcessLastAck (Ptr<Packet> packet, const TcpHeader& tcpHeader);

  // Window management

  /**
   * \brief Return count of number of unacked bytes
   * \returns count of number of unacked bytes
   */
  virtual uint32_t UnAckDataCount (void) const;

  /**
   * \brief Return total bytes in flight
   * \returns total bytes in flight
   */
  virtual uint32_t BytesInFlight (void);

  /**
   * \brief Return the max possible number of unacked bytes
   * \returns the max possible number of unacked bytes
   */
  virtual uint32_t Window (void) const;
  
  /**
   * \brief The amount of Rx window announced to the peer
   * \param scale indicate if the window should be scaled. True for
   * almost all cases, except when we are sending a SYN
   * \returns size of Rx window announced to the peer
   */
  virtual uint16_t AdvertisedWindowSize (bool scale = true) const;

  /**
   * \brief Update the receiver window (RWND) based on the value of the
   * window field in the header.
   *
   * This method suppresses updates unless one of the following three
   * conditions holds:  1) segment contains new data (advancing the right
   * edge of the receive buffer), 2) segment does not contain new data
   * but the segment acks new data (highest sequence number acked advances),
   * or 3) the advertised window is larger than the current send window
   *
   * \param header TcpHeader from which to extract the new window value
   * \return true if receiver window got updated
   */
  virtual bool UpdateWindowSize (const TcpHeader& header);


  // Manage data tx/rx

  /**
   * \brief Call CopyObject<> to clone me
   * \returns a copy of the socket
   */
  virtual Ptr<TcpSocketImpl> Fork (void) override;

  /**
   * \brief Received an ACK packet
   * \param packet the packet
   * \param tcpHeader the packet's TCP header
   */
  virtual void ReceivedAck (Ptr<Packet> packet, const TcpHeader& tcpHeader);

  /**
   * \brief Recv of a data, put into buffer, call L7 to get it if necessary
   * \param packet the packet
   * \param tcpHeader the packet's TCP header
   */
  virtual void ReceivedData (Ptr<Packet> packet, const TcpHeader& tcpHeader);

  /**
   * \brief Take into account the packet for RTT estimation
   * \param tcpHeader the packet's TCP header
   */
  virtual void EstimateRtt (const TcpHeader& tcpHeader);

  /**
   * \brief Update the RTT history, when we send TCP segments
   *
   * \param seq The sequence number of the TCP segment
   * \param sz The segment's size
   * \param isRetransmission Whether or not the segment is a retransmission
   */

  virtual void UpdateRttHistory (const SequenceNumber32 &seq, uint32_t sz,
                                 bool isRetransmission);

  /**
   * \brief Update buffers w.r.t. ACK
   * \param seq the sequence number
   * \param resetRTO indicates if RTO should be reset
   */
  virtual void NewAck (const TcpHeader& header, bool resetRTO);

  /**
   * \brief Call Retransmit() upon RTO event
   */
  virtual void ReTxTimeout (void);

  /**
   * \brief Halving cwnd and call DoRetransmit()
   */
  virtual void Retransmit (void);

  /**
   * \brief Action upon delay ACK timeout, i.e. send an ACK
   */
  virtual void DelAckTimeout (void);

  /**
   * \brief Timeout at LAST_ACK, close the connection
   */
  virtual void LastAckTimeout (void);

  /**
   * \brief Send 1 byte probe to get an updated window size
   */
  virtual void PersistTimeout (void);

  /**
   * \brief Retransmit the oldest packet
   */
  virtual void DoRetransmit (void);

  /**
   * \brief Called when a new ack arrvied
   */
  virtual void UpdateTxBuffer(SequenceNumber32 ack);
  
  /** \brief Add options to TcpHeader
   *
   * Test each option, and if it is enabled on our side, add it
   * to the header
   *
   * In linux, options are first enabled ,
   * then they are generated in tcp_options_write.
   *
   * \param tcpHeader TcpHeader to add options to
   */
  virtual void AddOptions (TcpHeader& tcpHeader);
  
  void ResetUserCallbacks (void);

  /**
   * Process an option before we call the main TCP functionality on receiving a segment
   */
  virtual void PreProcessOption(Ptr<const TcpOption> option);
  
  /**
   * Process an option after we call the main TCP functionality on receiving a segment
   */
  virtual void PostProcessOption(Ptr<const TcpOption> option);
  
  virtual void ProcessOptions (const TcpHeader& header, bool post);
  
  /**
   * \brief Read and parse the Window scale option
   *
   * Read the window scale option (encoded logarithmically) and save it.
   * Per RFC 1323, the value can't exceed 14.
   *
   * \param option Window scale option read from the header
   */
  virtual void ProcessOptionWScale (const Ptr<const TcpOption> option);
  
  /**
   * \brief Add the window scale option to the header
   *
   * Calculate our factor from the rxBuffer max size, and add it
   * to the header.
   *
   * \param header TcpHeader where the method should add the window scale option
   */
  virtual void AddOptionWScale (TcpHeader& header);
  
  /**
   * \brief Calculate window scale value based on receive buffer space
   *
   * Calculate our factor from the rxBuffer max size
   *
   * \returns the Window Scale factor
   */
  uint8_t CalculateWScale () const;

  /** \brief Process the timestamp option from other side
   *
   * Get the timestamp and the echo, then save timestamp (which will
   * be the echo value in our out-packets) and save the echoed timestamp,
   * to utilize later to calculate RTT.
   *
   * \see EstimateRtt
   * \param option Option from the segment
   * \param seq Sequence number of the segment
   */
  void ProcessOptionTimestamp (const Ptr<const TcpOption> option,
                               const SequenceNumber32 &seq);
  /**
   * \brief Add the timestamp option to the header
   *
   * Set the timestamp as the lower bits of the Simulator::Now time,
   * and the echo value as the last seen timestamp from the other part.
   *
   * \param header TcpHeader to which add the option to
   */
  void AddOptionTimestamp (TcpHeader& header);

  /**
   * \brief Scale the initial SsThresh value to the correct one
   *
   * Set the initial SsThresh to the largest possible advertised window
   * according to the sender scale factor.
   *
   * \param scaleFactor the sender scale factor
   */
  virtual void ScaleSsThresh (uint8_t scaleFactor);

  /**
   * \brief Initialize congestion window
   *
   * Default cWnd to 1 MSS (RFC2001, sec.1) and must
   * not be larger than 2 MSS (RFC2581, sec.3.1). Both m_initiaCWnd and
   * m_segmentSize are set by the attribute system in ns3::TcpSocket.
   */
  virtual void InitializeCwnd ();

  /**
   *
   */
  virtual Time ComputeRTO() const;

  /**
   *
   */
  virtual bool IsTcpOptionAllowed(uint8_t  kind) const;
  
  
  /**
   * \brief Performs a safe subtraction between a and b (a-b)
   *
   * Safe is used to indicate that, if b>a, the results returned is 0.
   *
   * \param a first number
   * \param b second number
   * \return 0 if b>0, (a-b) otherwise
   */
  static uint32_t SafeSubtraction (uint32_t a, uint32_t b);

  //!< TODO try to remove some friends
  friend class TcpTraceHelper;
  
  //Attribute set/get inherited from TcpSocket
  virtual void SetSegSize (uint32_t size) override;
  virtual uint32_t GetSegSize (void) const override;
  virtual void SetInitialSSThresh (uint32_t threshold) override;
  virtual uint32_t GetInitialSSThresh (void) const override;
  virtual void SetInitialCwnd (uint32_t cwnd) override;
  virtual uint32_t GetInitialCwnd (void) const override;
  
  virtual void NotifyRcvBufferChange (uint32_t oldSize, uint32_t newSize) override;
  virtual void SetSndBufSize (uint32_t size) override;
  virtual uint32_t GetSndBufSize (void) const override;
  virtual void SetRcvBufSize (uint32_t size) override;
  virtual uint32_t GetRcvBufSize (void) const override;
  
  //Helper method to set the state
  virtual void SetState (TcpStates_t aState);
  
  // Counters and events
  EventId           m_retxEvent;       //!< Retransmission event
  EventId           m_lastAckEvent;    //!< Last ACK timeout event
  EventId           m_delAckEvent;     //!< Delayed ACK timeout event
  EventId           m_persistEvent;    //!< Persist event: Send 1 byte to probe for a non-zero Rx window
  EventId           m_timewaitEvent;   //!< TIME_WAIT expiration event: Move this socket to CLOSED state
  uint32_t          m_dupAckCount;     //!< Dupack counter
  uint32_t          m_delAckCount;     //!< Delayed ACK counter
  uint32_t          m_synCount;        //!< Count of remaining connection retries
  uint32_t          m_dataRetrCount;   //!< Count of remaining data retransmission attempts
  TracedValue<Time> m_rto;             //!< Retransmit timeout
  
  TracedValue<Time> m_lastRtt;         //!< Last RTT sample collected
  
  RttHistory_t      m_history;         //!< List of sent packet

  // Connections to other layers of TCP/IP
  Ipv4EndPoint*       m_endPoint;   //!< the IPv4 endpoint
  Ipv6EndPoint*       m_endPoint6;  //!< the IPv6 endpoint
  
  Callback<void, Ipv4Address,uint8_t,uint8_t,uint8_t,uint32_t> m_icmpCallback;  //!< ICMP callback
  Callback<void, Ipv6Address,uint8_t,uint8_t,uint8_t,uint32_t> m_icmpCallback6; //!< ICMPv6 callback
  
  // State-related attributes
  TracedValue<TcpStates_t> m_state;         //!< TCP state
  mutable enum SocketErrno m_errno;         //!< Socket error code
  
  bool                     m_connected;     //!< Connection established
  
  // Window management
  TracedValue<uint32_t> m_rWnd;        //!< Receiver window (RCV.WND in RFC793)
  TracedValue<SequenceNumber32> m_highRxMark;     //!< Highest seqno received
  SequenceNumber32 m_highTxAck;                   //!< Highest ack sent
  TracedValue<SequenceNumber32> m_highRxAckMark;  //!< Highest ack received
  uint32_t                      m_bytesAckedNotProcessed;  //!< Bytes acked, but not processed
  TracedValue<uint32_t>         m_bytesInFlight; //!< Bytes in flight
  bool m_nullIsn;       //< Should the ISN be null ?
  
  uint8_t m_rcvWindShift;      //!< Window shift to apply to outgoing segments
  uint8_t m_sndWindShift;      //!< Window shift to apply to incoming segments
  
  uint32_t m_timestampToEcho;     //!< Timestamp to echo

  EventId m_sendPendingDataEvent; //!< micro-delay event to send pending data

  // Fast Retransmit and Recovery
  SequenceNumber32       m_recover;      //!< Previous highest Tx seqnum for fast recovery
  uint32_t               m_retransOut;   //!< Number of retransmissions in this window

  // Transmission Control Block
  Ptr<TcpSocketState>    m_tcb;               //!< Congestion control information
  
  // Rx and Tx buffer management
  Ptr<TcpRxBuffer32>        m_rxBuffer;       //!< Rx buffer (reordering buffer)
  Ptr<TcpTxBuffer32>        m_txBuffer;       //!< Tx buffer
  
  // Guesses over the other connection end
  bool m_isFirstPartialAck; //!< First partial ACK during RECOVERY
  
  // The following two traces pass a packet with a TCP header
  TracedCallback<Ptr<const Packet>, const TcpHeader&,
                 Ptr<const TcpSocketBase> > m_txTrace; //!< Trace of transmitted packets

  TracedCallback<Ptr<const Packet>, const TcpHeader&,
                 Ptr<const TcpSocketBase> > m_rxTrace; //!< Trace of received packets
};

/**
 * \ingroup tcp
 * TracedValue Callback signature for TcpCongState_t
 *
 * \param [in] oldValue original value of the traced variable
 * \param [in] newValue new value of the traced variable
 */
typedef void (* TcpCongStatesTracedValueCallback)(const TcpSocketState::TcpCongState_t oldValue,
const TcpSocketState::TcpCongState_t newValue);
  
#undef DISABLE_MEMBER
} // namespace ns3

#endif /* TCP_SOCKET_BASE_H */