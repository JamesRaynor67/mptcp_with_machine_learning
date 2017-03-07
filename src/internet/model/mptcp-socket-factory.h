/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 University College London
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
 * Author: Lynne Salameh <l.salameh@ucl.ac.uk>
 */
#ifndef MPTCP_SOCKET_FACTORY_H
#define MPTCP_SOCKET_FACTORY_H

#include "ns3/tcp-socket-factory.h"
#include "ns3/ptr.h"

namespace ns3 {

class TcpL4Protocol;

/**
 * \ingroup internet
 * \defgroup tcp Tcp
 *
 * Transmission Control Protocol
 *
 * See \RFC{793} and others.
 */

/**
 * \ingroup tcp
 *
 * \brief socket factory implementation for native ns-3 TCP
 *
 *
 * This class serves to create sockets of the TcpSocketBase type.
 */
class MpTcpSocketFactory : public SocketFactory
{
public:
  MpTcpSocketFactory ();
  virtual ~MpTcpSocketFactory ();
  
  /**
   * Get the type ID.
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Set the associated TCP L4 protocol.
   * \param tcp the TCP L4 protocol
   */
  void SetTcp (Ptr<TcpL4Protocol> tcp);

  virtual Ptr<Socket> CreateSocket (void);

protected:
  virtual void DoDispose (void);
private:
  Ptr<TcpL4Protocol> m_tcp; //!< the associated TCP L4 protocol
};

} // namespace ns3

#endif /* MPTCP_SOCKET_FACTORY_H */
