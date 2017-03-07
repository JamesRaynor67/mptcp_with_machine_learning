/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Author: Lynne Salameh <l.salameh@cs.ucl.ac.uk>
 *
 */

#ifndef TCP_SOCKET_WRAPPER_H
#define TCP_SOCKET_WRAPPER_H

#include "tcp-socket.h"
#include "ns3/socket.h"

namespace ns3
{

class TcpSocketWrapper : public Socket
{
public:
  
  TcpSocketWrapper ();
  ~TcpSocketWrapper ();
  
  static TypeId GetTypeId (void);
  
  virtual TypeId GetInstanceTypeId () const;
  
  void SetSocket (Ptr<TcpSocket> socket);
  
  Ptr<TcpSocket> GetSocket ();
  
  
  //Inherited from Socket Class, just pass these onto the enclosed tcp socket
  
  /**
   * \brief Get last error number.
   *
   * \return the errno associated to the last call which failed in this
   *         socket. Each socket's errno is initialized to zero
   *         when the socket is created.
   */
  virtual enum Socket::SocketErrno GetErrno (void) const;
  
  /**
   * \return the socket type, analogous to getsockopt (SO_TYPE)
   */
  virtual enum Socket::SocketType GetSocketType (void) const;
  /**
   * \brief Return the node this socket is associated with.
   * \returns the node
   */
  virtual Ptr<Node> GetNode (void) const;
  
  /**
   * \brief Specify callbacks to allow the caller to determine if
   * the connection succeeds of fails.
   * \param connectionSucceeded this callback is invoked when the
   *        connection request initiated by the user is successfully
   *        completed. The callback is passed  back a pointer to
   *        the same socket object.
   * \param connectionFailed this callback is invoked when the
   *        connection request initiated by the user is unsuccessfully
   *        completed. The callback is passed back a pointer to the
   *        same socket object.
   */
  virtual void SetConnectCallback (Callback<void, Ptr<Socket> > connectionSucceeded,
                           Callback<void,  Ptr<Socket> > connectionFailed);
  /**
   * \brief Detect socket recv() events such as graceful shutdown or error.
   *
   * For connection-oriented sockets, the first callback is used to signal
   * that the remote side has gracefully shut down the connection, and the
   * second callback denotes an error corresponding to cases in which
   * a traditional recv() socket call might return -1 (error), such
   * as a connection reset.  For datagram sockets, these callbacks may
   * never be invoked.
   *
   * \param normalClose this callback is invoked when the
   *        peer closes the connection gracefully
   * \param errorClose this callback is invoked when the
   *        connection closes abnormally
   */
  virtual void SetCloseCallbacks (Callback<void, Ptr<Socket> > normalClose,
                          Callback<void, Ptr<Socket> > errorClose);
  /**
   * \brief Accept connection requests from remote hosts
   * \param connectionRequest Callback for connection request from peer.
   *        This user callback is passed a pointer to this socket, the
   *        ip address and the port number of the connection originator.
   *        This callback must return true to accept the incoming connection,
   *        false otherwise. If the connection is accepted, the
   *        "newConnectionCreated" callback will be invoked later to
   *        give access to the user to the socket created to match
   *        this new connection. If the user does not explicitly
   *        specify this callback, all incoming  connections will be refused.
   * \param newConnectionCreated Callback for new connection: when a new
   *        is accepted, it is created and the corresponding socket is passed
   *        back to the user through this callback. This user callback is
   *        passed a pointer to the new socket, and the ip address and
   *        port number of the connection originator.
   */
  virtual void SetAcceptCallback (Callback<bool, Ptr<Socket>,
                          const Address &> connectionRequest,
                          Callback<void, Ptr<Socket>,
                          const Address&> newConnectionCreated);
  /**
   * \brief Notify application when a packet has been sent from transport
   *        protocol (non-standard socket call)
   * \param dataSent Callback for the event that data is sent from the
   *        underlying transport protocol.  This callback is passed a
   *        pointer to the socket, and the number of bytes sent.
   */
  virtual void SetDataSentCallback (Callback<void, Ptr<Socket>,
                            uint32_t> dataSent);
  /**
   * \brief Notify application when space in transmit buffer is added
   *
   *        This callback is intended to notify a
   *        socket that would have been blocked in a blocking socket model
   *        that space is available in the transmit buffer and that it
   *        can call Send() again.
   *
   * \param sendCb Callback for the event that the socket transmit buffer
   *        fill level has decreased.  This callback is passed a pointer to
   *        the socket, and the number of bytes available for writing
   *        into the buffer (an absolute value).  If there is no transmit
   *        buffer limit, a maximum-sized integer is always returned.
   */
  virtual void SetSendCallback (Callback<void, Ptr<Socket>, uint32_t> sendCb);
  /**
   * \brief Notify application when new data is available to be read.
   *
   *        This callback is intended to notify a socket that would
   *        have been blocked in a blocking socket model that data
   *        is available to be read.
   */
  virtual void SetRecvCallback (Callback<void, Ptr<Socket> >);
  /**
   * \brief Allocate a local endpoint for this socket.
   * \param address the address to try to allocate
   * \returns 0 on success, -1 on failure.
   */
  virtual int Bind (const Address &address);
  
  /**
   * \brief Allocate a local IPv4 endpoint for this socket.
   *
   * \returns 0 on success, -1 on failure.
   */
  virtual int Bind ();
  
  /**
   * \brief Allocate a local IPv6 endpoint for this socket.
   *
   * \returns 0 on success, -1 on failure.
   */
  virtual int Bind6 ();
  
  /**
   * \brief Close a socket.
   * \returns zero on success, -1 on failure.
   *
   * After the Close call, the socket is no longer valid, and cannot
   * safely be used for subsequent operations.
   */
  virtual int Close (void);
  
  /**
   * \returns zero on success, -1 on failure.
   *
   * Do not allow any further Send calls. This method is typically
   * implemented for Tcp sockets by a half close.
   */
  virtual int ShutdownSend (void);
  
  /**
   * \returns zero on success, -1 on failure.
   *
   * Do not allow any further Recv calls. This method is typically
   * implemented for Tcp sockets by a half close.
   */
  virtual int ShutdownRecv (void);
  
  /**
   * \brief Initiate a connection to a remote host
   * \param address Address of remote.
   * \returns 0 on success, -1 on error (in which case errno is set).
   */
  virtual int Connect (const Address &address);
  
  /**
   * \brief Listen for incoming connections.
   * \returns 0 on success, -1 on error (in which case errno is set).
   */
  virtual int Listen (void);
  
  /**
   * \brief Returns the number of bytes which can be sent in a single call
   * to Send.
   *
   * For datagram sockets, this returns the number of bytes that
   * can be passed atomically through the underlying protocol.
   *
   * For stream sockets, this returns the available space in bytes
   * left in the transmit buffer.
   *
   * \returns The number of bytes which can be sent in a single Send call.
   */
  virtual uint32_t GetTxAvailable (void) const;
  
  /**
   * \brief Send data (or dummy data) to the remote host
   *
   * This function matches closely in semantics to the send() function
   * call in the standard C library (libc):
   *   ssize_t send (int s, const void *msg, size_t len, int flags);
   * except that the send I/O is asynchronous.  This is the
   * primary Send method at this low-level API and must be implemented
   * by subclasses.
   *
   * In a typical blocking sockets model, this call would block upon
   * lack of space to hold the message to be sent.  In ns-3 at this
   * API, the call returns immediately in such a case, but the callback
   * registered with SetSendCallback() is invoked when the socket
   * has space (when it conceptually unblocks); this is an asynchronous
   * I/O model for send().
   *
   * This variant of Send() uses class ns3::Packet to encapsulate
   * data, rather than providing a raw pointer and length field.
   * This allows an ns-3 application to attach tags if desired (such
   * as a flow ID) and may allow the simulator to avoid some data
   * copies.  Despite the appearance of sending Packets on a stream
   * socket, just think of it as a fancy byte buffer with streaming
   * semantics.
   *
   * If either the message buffer within the Packet is too long to pass
   * atomically through the underlying protocol (for datagram sockets),
   * or the message buffer cannot entirely fit in the transmit buffer
   * (for stream sockets), -1 is returned and SocketErrno is set
   * to ERROR_MSGSIZE.  If the packet does not fit, the caller can
   * split the Packet (based on information obtained from
   * GetTxAvailable) and reattempt to send the data.
   *
   * The flags argument is formed by or'ing one or more of the values:
   *        MSG_OOB        process out-of-band data
   *        MSG_DONTROUTE  bypass routing, use direct interface
   * These flags are _unsupported_ as of ns-3.1.
   *
   * \param p ns3::Packet to send
   * \param flags Socket control flags
   * \returns the number of bytes accepted for transmission if no error
   *          occurs, and -1 otherwise.
   *
   * \see SetSendCallback
   */
  virtual int Send (Ptr<Packet> p, uint32_t flags);
  
  /**
   * \brief Send data to a specified peer.
   *
   * This method has similar semantics to Send () but subclasses may
   * want to provide checks on socket state, so the implementation is
   * pushed to subclasses.
   *
   * \param p packet to send
   * \param flags Socket control flags
   * \param toAddress IP Address of remote host
   * \returns -1 in case of error or the number of bytes copied in the
   *          internal buffer and accepted for transmission.
   */
  virtual int SendTo (Ptr<Packet> p, uint32_t flags,
                      const Address &toAddress);
  
  /**
   * Return number of bytes which can be returned from one or
   * multiple calls to Recv.
   * Must be possible to call this method from the Recv callback.
   *
   * \returns the number of bytes which can be returned from one or
   *          multiple Recv calls.
   */
  virtual uint32_t GetRxAvailable (void) const;
  
  /**
   * \brief Read data from the socket
   *
   * This function matches closely in semantics to the recv() function
   * call in the standard C library (libc):
   *   ssize_t recv (int s, void *buf, size_t len, int flags);
   * except that the receive I/O is asynchronous.  This is the
   * primary Recv method at this low-level API and must be implemented
   * by subclasses.
   *
   * This method is normally used only on a connected socket.
   * In a typical blocking sockets model, this call would block until
   * at least one byte is returned or the connection closes.
   * In ns-3 at this API, the call returns immediately in such a case
   * and returns 0 if nothing is available to be read.
   * However, an application can set a callback, ns3::SetRecvCallback,
   * to be notified of data being available to be read
   * (when it conceptually unblocks); this is an asynchronous
   * I/O model for recv().
   *
   * This variant of Recv() uses class ns3::Packet to encapsulate
   * data, rather than providing a raw pointer and length field.
   * This allows an ns-3 application to attach tags if desired (such
   * as a flow ID) and may allow the simulator to avoid some data
   * copies.  Despite the appearance of receiving Packets on a stream
   * socket, just think of it as a fancy byte buffer with streaming
   * semantics.
   *
   * The semantics depend on the type of socket.  For a datagram socket,
   * each Recv() returns the data from at most one Send(), and order
   * is not necessarily preserved.  For a stream socket, the bytes
   * are delivered in order, and on-the-wire packet boundaries are
   * not preserved.
   *
   * The flags argument is formed by or'ing one or more of the values:
   *        MSG_OOB             process out-of-band data
   *        MSG_PEEK            peek at incoming message
   * None of these flags are supported for now.
   *
   * Some variants of Recv() are supported as additional API,
   * including RecvFrom(), overloaded Recv() without arguments,
   * and variants that use raw character buffers.
   *
   * \param maxSize reader will accept packet up to maxSize
   * \param flags Socket control flags
   * \returns Ptr<Packet> of the next in-sequence packet.  Returns
   * 0 if the socket cannot return a next in-sequence packet conforming
   * to the maxSize and flags.
   *
   * \see SetRecvCallback
   */
  virtual Ptr<Packet> Recv (uint32_t maxSize, uint32_t flags);
  
  /**
   * \brief Read a single packet from the socket and retrieve the sender
   * address.
   *
   * Calls Recv(maxSize, flags) with maxSize
   * implicitly set to maximum sized integer, and flags set to zero.
   *
   * This method has similar semantics to Recv () but subclasses may
   * want to provide checks on socket state, so the implementation is
   * pushed to subclasses.
   *
   * \param maxSize reader will accept packet up to maxSize
   * \param flags Socket control flags
   * \param fromAddress output parameter that will return the
   * address of the sender of the received packet, if any.  Remains
   * untouched if no packet is received.
   * \returns Ptr<Packet> of the next in-sequence packet.  Returns
   * 0 if the socket cannot return a next in-sequence packet.
   */
  virtual Ptr<Packet> RecvFrom (uint32_t maxSize, uint32_t flags,
                                Address &fromAddress);
  
  /////////////////////////////////////////////////////////////////////
  //   The remainder of these public methods are overloaded methods  //
  //   or variants of Send() and Recv(), and they are non-virtual    //
  /////////////////////////////////////////////////////////////////////
  
  /**
   * \brief Get socket address.
   * \param address the address name this socket is associated with.
   * \returns 0 if success, -1 otherwise
   */
  virtual int GetSockName (Address &address) const;
  
  /**
   * \brief Get the peer address of a connected socket.
   * \param address the address this socket is connected to.
   * \returns 0 if success, -1 otherwise
   */
  virtual int GetPeerName (Address &address) const;
  
  /**
   * \brief Bind a socket to specific device.
   *
   * This method corresponds to using setsockopt() SO_BINDTODEVICE
   * of real network or BSD sockets.   If set on a socket, this option will
   * force packets to leave the bound device regardless of the device that
   * IP routing would naturally choose.  In the receive direction, only
   * packets received from the bound interface will be delivered.
   *
   * This option has no particular relationship to binding sockets to
   * an address via Socket::Bind ().  It is possible to bind sockets to a
   * specific IP address on the bound interface by calling both
   * Socket::Bind (address) and Socket::BindToNetDevice (device), but it
   * is also possible to bind to mismatching device and address, even if
   * the socket can not receive any packets as a result.
   *
   * \warning BindToNetDevice should be used \a after Bind. Otherwise
   * it will perform a Bind itself.
   *
   * \param netdevice Pointer to Netdevice of desired interface
   * \returns nothing
   */
  virtual void BindToNetDevice (Ptr<NetDevice> netdevice);
  
  /**
   * \brief Returns socket's bound netdevice, if any.
   *
   * This method corresponds to using getsockopt() SO_BINDTODEVICE
   * of real network or BSD sockets.
   *
   *
   * \returns Pointer to interface.
   */
  Ptr<NetDevice> GetBoundNetDevice ();
  
  
  /**
   * \brief Configure whether broadcast datagram transmissions are allowed
   *
   * This method corresponds to using setsockopt() SO_BROADCAST of
   * real network or BSD sockets.  If set on a socket, this option
   * will enable or disable packets to be transmitted to broadcast
   * destination addresses.
   *
   * \param allowBroadcast Whether broadcast is allowed
   * \return true if operation succeeds
   */
  virtual bool SetAllowBroadcast (bool allowBroadcast);
  
  /**
   * \brief Query whether broadcast datagram transmissions are allowed
   *
   * This method corresponds to using getsockopt() SO_BROADCAST of
   * real network or BSD sockets.
   *
   * \returns true if broadcast is allowed, false otherwise
   */
  virtual bool GetAllowBroadcast () const;
  
  /**
   * \brief Enable/Disable receive packet information to socket.
   *
   * For IP_PKTINFO/IP6_PKTINFO. This method is only usable for
   * Raw socket and Datagram Socket. Not supported for Stream socket.
   *
   * Method doesn't make distinction between IPv4 and IPv6. If it is enabled,
   * it is enabled for all types of sockets that supports packet information
   *
   * \param flag Enable/Disable receive information
   * \returns nothing
   */
  virtual void SetRecvPktInfo (bool flag);
  
  /**
   * \brief Get status indicating whether enable/disable packet information to socket
   *
   * \returns True if packet information should be sent to socket
   */
  virtual bool IsRecvPktInfo () const;
  
  /**
   * \brief Manually set the socket priority
   *
   * This method corresponds to using setsockopt () SO_PRIORITY of
   * real network or BSD sockets.
   *
   * \param priority The socket priority (in the range 0..6)
   */
  virtual void SetPriority (uint8_t priority);
  
  /**
   * \brief Query the priority value of this socket
   *
   * This method corresponds to using getsockopt () SO_PRIORITY of real network
   * or BSD sockets.
   *
   * \return The priority value
   */
  virtual uint8_t GetPriority (void) const;
  
  /**
   * \brief Manually set IP Type of Service field
   *
   * This method corresponds to using setsockopt () IP_TOS of
   * real network or BSD sockets. This option is for IPv4 only.
   * Setting the IP TOS should also change the socket queueing
   * priority as stated in the man page. However, socket priority
   * is not yet supported.
   *
   * \param ipTos The desired TOS value for IP headers
   */
  virtual void SetIpTos (uint8_t ipTos);
  
  /**
   * \brief Query the value of IP Type of Service of this socket
   *
   * This method corresponds to using getsockopt () IP_TOS of real network
   * or BSD sockets.
   *
   * \return The raw IP TOS value
   */
  virtual uint8_t GetIpTos (void) const;
  
  /**
   * \brief Tells a socket to pass information about IP Type of Service up the stack
   *
   * This method corresponds to using setsockopt () IP_RECVTOS of real
   * network or BSD sockets. In our implementation, the socket simply
   * adds a SocketIpTosTag tag to the packet before passing the
   * packet up the stack.
   *
   * \param ipv4RecvTos Whether the socket should add SocketIpv4TosTag tag
   * to the packet
   */
  virtual void SetIpRecvTos (bool ipv4RecvTos);
  
  /**
   * \brief Ask if the socket is currently passing information about IP Type of Service up the stack
   *
   * This method corresponds to using getsockopt () IP_RECVTOS of real
   * network or BSD sockets.
   *
   * \return Whether the IP_RECVTOS is set
   */
  virtual bool IsIpRecvTos (void) const;
  
  /**
   * \brief Manually set IPv6 Traffic Class field
   *
   * This method corresponds to using setsockopt () IPV6_TCLASS of
   * real network or BSD sockets. This option is for IPv6 only.
   * Setting the IPV6_TCLASSS to -1 clears the option and let the socket
   * uses the default value.
   *
   * \param ipTclass The desired TCLASS value for IPv6 headers
   */
  virtual void SetIpv6Tclass (int ipTclass);
  
  /**
   * \brief Query the value of IPv6 Traffic Class field of this socket
   *
   * This method corresponds to using getsockopt () IPV6_TCLASS of real network
   * or BSD sockets.
   *
   * \return The raw IPV6_TCLASS value
   */
  virtual uint8_t GetIpv6Tclass (void) const;
  
  /**
   * \brief Tells a socket to pass information about IPv6 Traffic Class up the stack
   *
   * This method corresponds to using setsockopt () IPV6_RECVTCLASS of real
   * network or BSD sockets. In our implementation, the socket simply
   * adds a SocketIpv6TclasssTag tag to the packet before passing the
   * packet up the stack.
   *
   * \param ipv6RecvTclass Whether the socket should add SocketIpv6TclassTag tag
   * to the packet
   */
  virtual void SetIpv6RecvTclass (bool ipv6RecvTclass);
  
  /**
   * \brief Ask if the socket is currently passing information about IPv6 Traffic Class up the stack
   *
   * This method corresponds to using getsockopt () IPV6_RECVTCLASS of real
   * network or BSD sockets.
   *
   * \return Whether the IPV6_RECVTCLASS is set
   */
  virtual bool IsIpv6RecvTclass (void) const;
  
  /**
   * \brief Manually set IP Time to Live field
   *
   * This method corresponds to using setsockopt () IP_TTL of
   * real network or BSD sockets.
   *
   * \param ipTtl The desired TTL value for IP headers
   */
  virtual void SetIpTtl (uint8_t ipTtl);
  
  /**
   * \brief Query the value of IP Time to Live field of this socket
   *
   * This method corresponds to using getsockopt () IP_TTL of real network
   * or BSD sockets.
   *
   * \return The raw IP TTL value
   */
  virtual uint8_t GetIpTtl (void) const;
  
  /**
   * \brief Tells a socket to pass information about IP_TTL up the stack
   *
   * This method corresponds to using setsockopt () IP_RECVTTL of real
   * network or BSD sockets. In our implementation, the socket simply
   * adds a SocketIpTtlTag tag to the packet before passing the
   * packet up the stack.
   *
   * \param ipv4RecvTtl Whether the socket should add SocketIpv4TtlTag tag
   * to the packet
   */
  virtual void SetIpRecvTtl (bool ipv4RecvTtl);
  
  /**
   * \brief Ask if the socket is currently passing information about IP_TTL up the stack
   *
   * This method corresponds to using getsockopt () IP_RECVTTL of real
   * network or BSD sockets.
   *
   * \return Whether the IP_RECVTTL is set
   */
  virtual bool IsIpRecvTtl (void) const;
  
  /**
   * \brief Manually set IPv6 Hop Limit
   *
   * This method corresponds to using setsockopt () IPV6_HOPLIMIT of
   * real network or BSD sockets.
   *
   * \param ipHopLimit The desired Hop Limit value for IPv6 headers
   */
  virtual void SetIpv6HopLimit (uint8_t ipHopLimit);
  
  /**
   * \brief Query the value of IP Hop Limit field of this socket
   *
   * This method corresponds to using getsockopt () IPV6_HOPLIMIT of real network
   * or BSD sockets.
   *
   * \return The raw IPv6 Hop Limit value
   */
  virtual uint8_t GetIpv6HopLimit (void) const;
  
  /**
   * \brief Tells a socket to pass information about IPv6 Hop Limit up the stack
   *
   * This method corresponds to using setsockopt () IPV6_RECVHOPLIMIT of real
   * network or BSD sockets. In our implementation, the socket simply
   * adds a SocketIpv6HopLimitTag tag to the packet before passing the
   * packet up the stack.
   *
   * \param ipv6RecvHopLimit Whether the socket should add SocketIpv6HopLimitTag tag
   * to the packet
   */
  virtual void SetIpv6RecvHopLimit (bool ipv6RecvHopLimit);
  
  /**
   * \brief Ask if the socket is currently passing information about IPv6 Hop Limit up the stack
   *
   * This method corresponds to using getsockopt () IPV6_RECVHOPLIMIT of real
   * network or BSD sockets.
   *
   * \return Whether the IPV6_RECVHOPLIMIT is set
   */
  virtual bool IsIpv6RecvHopLimit (void) const;
  
  /**
   * \brief Joins a IPv6 multicast group.
   *
   * Based on the filter mode and source addresses this can be interpreted as a
   * join, leave, or modification to source filtering on a multicast group.
   *
   * Mind that a socket can join only one multicast group. Any attempt to join another group will remove the old one.
   *
   *
   * \param address Requested multicast address.
   * \param filterMode Socket filtering mode (INCLUDE | EXCLUDE).
   * \param sourceAddresses All the source addresses on which socket is interested or not interested.
   */
  virtual void Ipv6JoinGroup (Ipv6Address address, Ipv6MulticastFilterMode filterMode, std::vector<Ipv6Address> sourceAddresses);
  
  /**
   * \brief Joins a IPv6 multicast group without filters.
   *
   * A socket can join only one multicast group. Any attempt to join another group will remove the old one.
   *
   * \param address Group address on which socket wants to join.
   */
  virtual void Ipv6JoinGroup (Ipv6Address address);
  
  /**
   * \brief Leaves IPv6 multicast group this socket is joined to.
   */
  virtual void Ipv6LeaveGroup (void);
  
protected:
  
  /**
   * \brief Checks if the socket has a specific IPv6 Tclass set
   *
   * \returns true if the socket has a IPv6 Tclass set, false otherwise.
   */
  virtual bool IsManualIpv6Tclass (void) const;
  
  /**
   * \brief Checks if the socket has a specific IPv4 TTL set
   *
   * \returns true if the socket has a IPv4 TTL set, false otherwise.
   */
  virtual bool IsManualIpTtl (void) const;
  
  /**
   * \brief Checks if the socket has a specific IPv6 Hop Limit set
   *
   * \returns true if the socket has a IPv6 Hop Limit set, false otherwise.
   */
  virtual bool IsManualIpv6HopLimit (void) const;
  
private:
  
  Ptr<TcpSocket> m_socket;
  
};
  
}

#endif /* TCP_SOCKET_WRAPPER_H */
