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
 */


#ifndef TCP_SOCKET_IMPL_H
#define TCP_SOCKET_IMPL_H

#include "ns3/tcp-socket.h"
#include "tcp-l4-protocol.h"
#include "tcp-parameters.h"
#include "ns3/rtt-estimator.h"
#include "ns3/tcp-rx-buffer.h"
#include "ns3/tcp-tx-buffer.h"


namespace ns3
{
  class TcpCongestionOps;
  
  class TcpSocketImpl : public TcpSocket
  {
  public:
    TcpSocketImpl ();
    TcpSocketImpl (const TcpSocketImpl& sock);
    ~TcpSocketImpl ();
    
    static TypeId GetTypeId (void);
    virtual TypeId GetInstanceTypeId (void) const override;
    
    //To be called from MptcpL4Protocol
    virtual void SetNode (Ptr<Node> node);
    virtual Ptr<Node> GetNode (void) const override;
    virtual void SetTcp (Ptr<TcpL4Protocol> tcp);
    virtual void SetCongestionControlAlgorithm (Ptr<TcpCongestionOps> algo);
    virtual void SetRtt (Ptr<RttEstimator> rtt);
    virtual Ptr<const RttEstimator> GetRttEstimator();
    
    virtual bool SetAllowBroadcast (bool allowBroadcast) override;
    virtual bool GetAllowBroadcast () const override;
    
    virtual enum Socket::SocketType GetSocketType (void) const override;
    
    /**
     * \brief Sets the Minimum RTO.
     * \param minRto The minimum RTO.
     */
    void SetMinRto (Time minRto);
    
    /**
     * \brief Get the Minimum RTO.
     * \return The minimum RTO.
     */
    Time GetMinRto (void) const;
    
    /**
     * \brief Sets the Clock Granularity (used in RTO calcs).
     * \param clockGranularity The Clock Granularity
     */
    void SetClockGranularity (Time clockGranularity);
    
    /**
     * \brief Get the Clock Granularity (used in RTO calcs).
     * \return The Clock Granularity.
     */
    Time GetClockGranularity (void) const;
    
    virtual void CompleteFork(Ptr<Packet> p,
                              const TcpHeader& h,
                              const Address& fromAddress,
                              const Address& toAddress) = 0;
    
  protected:
    
    Ptr<const TcpParameters> GetTcpParameters () const;
    void SetTcpParameters (Ptr<const TcpParameters> params);
    
    //Inherited from TcpSocket, setting the TCP parameters
    
    virtual void NotifyRcvBufferChange (uint32_t oldSize, uint32_t newSize) = 0;
    
    
    /**
     * \brief Set the connection timeout.
     * \param timeout the connection timeout
     */
    virtual void SetConnTimeout (Time timeout) override;
    
    /**
     * \brief Get the connection timeout.
     * \returns the connection timeout
     */
    virtual Time GetConnTimeout (void) const override;
    
    /**
     * \brief Set the number of connection retries before giving up.
     * \param count the number of connection retries
     */
    virtual void SetSynRetries (uint32_t count) override;
    
    /**
     * \brief Get the number of connection retries before giving up.
     * \returns the number of connection retries
     */
    virtual uint32_t GetSynRetries (void) const override;
    
    /**
     * \brief Set the number of data transmission retries before giving up.
     * \param retries the number of data transmission retries
     */
    virtual void SetDataRetries (uint32_t retries) override;
    
    /**
     * \brief Get the number of data transmission retries before giving up.
     * \returns the number of data transmission retries
     */
    virtual uint32_t GetDataRetries (void) const override;
    
    /**
     * \brief Set the time to delay an ACK.
     * \param timeout the time to delay an ACK
     */
    virtual void SetDelAckTimeout (Time timeout) override;
    
    /**
     * \brief Get the time to delay an ACK.
     * \returns the time to delay an ACK
     */
    virtual Time GetDelAckTimeout (void) const override;
    
    /**
     * \brief Set the number of packet to fire an ACK before delay timeout.
     * \param count the umber of packet to fire an ACK before delay timeout
     */
    virtual void SetDelAckMaxCount (uint32_t count) override;
    
    /**
     * \brief Get the number of packet to fire an ACK before delay timeout.
     * \returns the number of packet to fire an ACK before delay timeout
     */
    virtual uint32_t GetDelAckMaxCount (void) const override;
    
    /**
     * \brief Enable/Disable Nagle's algorithm.
     * \param noDelay true to DISABLE Nagle's algorithm
     */
    virtual void SetTcpNoDelay (bool noDelay) override;
    
    /**
     * \brief Check if Nagle's algorithm is enabled or not.
     * \returns true if Nagle's algorithm is DISABLED
     */
    virtual bool GetTcpNoDelay (void) const override;
    
    /**
     * \brief Set the timout for persistent connection
     *
     * When the timout expires, send 1-byte data to probe for the window
     * size at the receiver when the local knowledge tells that the
     * receiver has zero window size
     *
     * \param timeout the persistent timout
     */
    virtual void SetPersistTimeout (Time timeout) override;
    
    /**
     * \brief Get the timout for persistent connection
     *
     * When the timout expires, send 1-byte data to probe for the window
     * size at the receiver when the local knowledge tells that the
     * receiver has zero window size
     *
     * \returns the persistent timout
     */
    virtual Time GetPersistTimeout (void) const override;
    
    virtual void SetMaxSegLifetime (double msl);
    virtual double GetMaxSegLifetime () const;
    
    virtual void SetMaxWindowSize (uint16_t size);
    virtual uint16_t GetMaxWindowSize () const;
    
    virtual void SetNullIsn (bool flag);
    virtual bool GetNullIsn () const;
    
    virtual void SetMptcpEnabled (bool flag);
    virtual bool GetMptcpEnabled () const;
    
    virtual void SetWindowScalingEnabled (bool flag);
    virtual bool GetWindowScalingEnabled () const;
    
    virtual void SetTimestampEnabled (bool flag);
    virtual bool GetTimestampEnabled () const;
    
    virtual void SetReTxThreshold (uint32_t thresh);
    virtual uint32_t GetReTxThreshold () const;
    
    virtual void SetLimitedTransmit (bool flag);
    virtual bool GetLimitedTransmit () const;
    
    /**
     * \brief Call CopyObject<> to clone me
     * \returns a copy of the socket
     */
    virtual Ptr<TcpSocketImpl> Fork (void) = 0;
    
    
    Ptr<TcpParameters>      m_tcpParams;
    
    Ptr<Node>               m_node;       //!< the associated node
    Ptr<TcpL4Protocol>      m_tcp;        //!< the associated TCP L4 protocol
    Ptr<TcpCongestionOps>   m_congestionControl;    //!< Congestion control
    Ptr<RttEstimator>       m_rtt; //!< Round trip time estimator
    
  };
}

#endif /* TCP_SOCKET_IMPL_H */
