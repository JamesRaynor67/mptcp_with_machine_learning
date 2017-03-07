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

#ifndef TCP_SOCKET_STATE_H
#define TCP_SOCKET_STATE_H

#include "ns3/object.h"
#include "ns3/traced-value.h"
#include "ns3/sequence-number.h"

namespace ns3 {
  
class TcpSocketImpl;

class TcpSocketState : public Object
{
public:
  /**
   * Get the type ID.
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  
  TcpSocketState ();
  
  /**
   * \brief Copy constructor.
   * \param other object to copy.
   */
  TcpSocketState (const TcpSocketState &other);
  
  /**
   * \brief Definition of the Congestion state machine
   *
   * The design of this state machine is taken from Linux v4.0, but it has been
   * maintained in the Linux mainline from ages. It basically avoids to maintain
   * a lot of boolean variables, and it allows to check the transitions from
   * different algorithm in a cleaner way.
   *
   * These states represent the situation from a congestion control point of view:
   * in fact, apart the CA_OPEN state, the other states represent a situation in
   * which there is a congestion, and different actions should be taken,
   * depending on the case.
   *
   */
  typedef enum
  {
    CA_OPEN,      /**< Normal state, no dubious events */
    CA_DISORDER,  /**< In all the respects it is "Open",
                   *  but requires a bit more attention. It is entered when
                   *  we see some SACKs or dupacks. It is split of "Open" */
    CA_CWR,       /**< cWnd was reduced due to some Congestion Notification event.
                   *  It can be ECN, ICMP source quench, local device congestion.
                   *  Not used in NS-3 right now. */
    CA_RECOVERY,  /**< CWND was reduced, we are fast-retransmitting. */
    CA_LOSS,      /**< CWND was reduced due to RTO timeout or SACK reneging. */
    CA_LAST_STATE /**< Used only in debug messages */
  } TcpCongState_t;
  
  /**
   * \ingroup tcp
   * TracedValue Callback signature for TcpCongState_t
   *
   * \param [in] oldValue original value of the traced variable
   * \param [in] newValue new value of the traced variable
   */
  typedef void (* TcpCongStatesTracedValueCallback)(const TcpCongState_t oldValue,
                      const TcpCongState_t newValue);
  
  /**
   * \brief Literal names of TCP states for use in log messages
   */
  static const char* const TcpCongStateName[TcpSocketState::CA_LAST_STATE];
  
  // Congestion control
  TracedValue<uint32_t>  m_cWnd;            //!< Congestion window
  TracedValue<uint32_t>  m_ssThresh;        //!< Slow start threshold
  uint32_t               m_initialCWnd;     //!< Initial cWnd value
  uint32_t               m_initialSsThresh; //!< Initial Slow Start Threshold value
  
  // Segment
  uint32_t               m_segmentSize;     //!< Segment size
  SequenceNumber32       m_lastAckedSeq;    //!< Last sequence ACKed
  
  Ptr<TcpSocketImpl>     m_socket;          //!< Pointer to socket, usually Null
  
  TracedValue<TcpCongState_t> m_congState;    //!< State in the Congestion state machine
  TracedValue<SequenceNumber32> m_highTxMark; //!< Highest seqno ever sent, regardless of ReTx
  TracedValue<SequenceNumber32> m_nextTxSequence; //!< Next seqnum to be sent (SND.NXT), ReTx pushes it back
  
  /**
   * \brief Get cwnd in segments rather than bytes
   *
   * \return Congestion window in segments
   */
  uint32_t GetCwndInSegments () const
  {
    return m_cWnd / m_segmentSize;
  }
  
  /**
   * \brief Get slow start thresh in segments rather than bytes
   *
   * \return Slow start threshold in segments
   */
  uint32_t GetSsThreshInSegments () const
  {
    return m_ssThresh / m_segmentSize;
  }
};
  
}
#endif /* TCP_SOCKET_STATE_H */
