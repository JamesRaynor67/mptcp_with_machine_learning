/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *
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
 */


#ifndef TCP_PARAMETERS_H
#define TCP_PARAMETERS_H

#include "ns3/simple-ref-count.h"
#include "ns3/nstime.h"

namespace ns3
{
  //Class to encapsulate the TCP parameters, to allow for easy copying of objects
  class TcpParameters : public SimpleRefCount<TcpParameters>
  {
  public:
    
    TcpParameters ();
    TcpParameters (const TcpParameters& params);
    
    ~TcpParameters ();
    
    /*
     *Socket attributes:
     */
    
    double            m_msl;              //!< Max segment lifetime
    bool              m_nullIsn;          //!< Should the initial sequence number be null ?
    Time              m_minRto;           //!< Minimum value of the Retransmit timeout
    Time              m_clockGranularity; //!< Clock Granularity used in RTO calcs
    // Fast retransmit and recovery
    uint32_t          m_retxThresh;       //!< Fast Retransmit threshold
    bool              m_limitedTx;        //!< perform limited transmit
    // Window Management
    uint16_t          m_maxWinSize;       //!< Maximum window size to advertise
    
    // Tcp Header Options
    bool              m_mptcpEnabled;       //!< MPTCP Enabled
    bool              m_winScalingEnabled;  //!< Window Scale option enabled (RFC 7323)
    bool              m_timestampEnabled;   //!< Timestamp option enabled
    
    //Properties from TcpSocket
    Time              m_cnTimeout;       //!< Timeout for connection retry
    uint32_t          m_synRetries;      //!< Number of connection attempts
    uint32_t          m_dataRetries;     //!< Number of data retransmission attempts
    Time              m_delAckTimeout;   //!< Time to delay an ACK
    uint32_t          m_delAckMaxCount;  //!< Number of packet to fire an ACK before delay timeout
    bool              m_noDelay;         //!< Set to true to disable Nagle's algorithm
    Time              m_persistTimeout;  //!< Time between sending 1-byte probes
    
    bool              m_closeNotified; //!< Told app to close socket
    bool              m_closeOnEmpty;  //!< Close socket upon tx buffer emptied
    bool              m_shutdownSend;  //!< Send no longer allowed
    bool              m_shutdownRecv;  //!< Receive no longer allowed
  };

}

#endif /* TCP_PARAMETERS_H */
