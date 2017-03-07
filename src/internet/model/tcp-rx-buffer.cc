/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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

#include "ns3/packet.h"
#include "ns3/fatal-error.h"
#include "ns3/log.h"
#include "tcp-rx-buffer.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpRxBuffer");



template<>
TypeId
TcpRxBuffer<uint32_t, int32_t>::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpRxBuffer32")
    .SetParent<Object> ()
    .SetGroupName ("Internet")
    .AddConstructor<TcpRxBuffer32> ()
    .AddTraceSource ("NextRxSequence",
                     "Next sequence number expected (RCV.NXT)",
                     MakeTraceSourceAccessor (&TcpRxBuffer::m_nextRxSeq),
                     "ns3::SequenceNumber32TracedValueCallback")
  ;
  return tid;
}
  
NS_OBJECT_ENSURE_REGISTERED (TcpRxBuffer32);
  
template<>
TypeId
TcpRxBuffer<uint64_t, int64_t>::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpRxBuffer64")
  .SetParent<Object> ()
  .SetGroupName ("Internet")
  .AddConstructor<TcpRxBuffer64> ()
  .AddTraceSource ("NextRxSequence",
                   "Next sequence number expected (RCV.NXT)",
                   MakeTraceSourceAccessor (&TcpRxBuffer::m_nextRxSeq),
                   "ns3::SequenceNumber64TracedValueCallback")
  ;
  return tid;
}

NS_OBJECT_ENSURE_REGISTERED (TcpRxBuffer64);

/* A user is supposed to create a TcpSocket through a factory. In TcpSocket,
 * there are attributes SndBufSize and RcvBufSize to control the default Tx and
 * Rx window sizes respectively, with default of 128 KiByte. The attribute
 * RcvBufSize is passed to TcpRxBuffer by TcpSocketBase::SetRcvBufSize() and in
 * turn, TcpRxBuffer:SetMaxBufferSize(). Therefore, the m_maxBuffer value
 * initialized below is insignificant.
 */
template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
TcpRxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::TcpRxBuffer (NUMERIC_TYPE n)
  : m_nextRxSeq (n), m_gotFin (false), m_size (0), m_maxBuffer (32768), m_availBytes (0)
{
}

template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
TcpRxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::~TcpRxBuffer ()
{
}

template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE>
TcpRxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::NextRxSequence (void) const
{
  return m_nextRxSeq;
}

template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
void
TcpRxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::SetNextRxSequence (const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE>& s)
{
  m_nextRxSeq = s;
}
  
template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE>
TcpRxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::HeadSequence(void) const
{
  return NextRxSequence()-Available();
}

template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
void
TcpRxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::Dump() const
{
  NS_LOG_DEBUG("=== Dumping content of RxBuffer");
  NS_LOG_DEBUG("> nextRxSeq=" << m_nextRxSeq << " Occupancy=" << m_size);
  BufConstIterator i = m_data.begin ();
  for( ; i != m_data.end (); ++i)
  {
    NS_LOG_DEBUG( "head:" << i->first << " of size:" << i->second->GetSize());
  }
  NS_LOG_DEBUG("=== End of dump");
}

template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
uint32_t
TcpRxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::MaxBufferSize (void) const
{
  return m_maxBuffer;
}

template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
void
TcpRxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::SetMaxBufferSize (uint32_t s)
{
  m_maxBuffer = s;
}

template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
uint32_t
TcpRxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::Size (void) const
{
  return m_size;
}

template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
uint32_t
TcpRxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::Available () const
{
  return m_availBytes;
}

template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
void
TcpRxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::IncNextRxSequence ()
{
  NS_LOG_FUNCTION (this);
  // Increment nextRxSeq is valid only if we don't have any data buffered,
  // this is supposed to be called only during the three-way handshake
  NS_ASSERT (m_size == 0);
  m_nextRxSeq++;
}

// Return the lowest sequence number that this TcpRxBuffer cannot accept
template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE>
TcpRxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::MaxRxSequence (void) const
{
  if (m_gotFin)
    { // No data allowed beyond FIN
      return m_finSeq;
    }
  else if (m_data.size ())
    { // No data allowed beyond Rx window allowed
      return m_data.begin ()->first + SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> (m_maxBuffer);
    }
  return m_nextRxSeq + SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE>(m_maxBuffer);
}

template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
void
TcpRxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::SetFinSequence (const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE>& s)
{
  NS_LOG_FUNCTION (this);

  m_gotFin = true;
  m_finSeq = s;
  if (m_nextRxSeq == m_finSeq) ++m_nextRxSeq;
}

template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
bool
TcpRxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::Finished (void)
{
  return (m_gotFin && m_finSeq < m_nextRxSeq);
}

template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
bool
TcpRxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::Add (Ptr<Packet> p, SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> headSeq)
{
  NS_LOG_FUNCTION (this << p << headSeq);

  uint32_t pktSize = p->GetSize ();
  SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> prevHeadSeq = headSeq;
  SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> tailSeq = headSeq + SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> (pktSize);
  NS_LOG_LOGIC ("Add pkt " << p << " len=" << pktSize << " seq=" << headSeq
                           << ", when NextRxSeq=" << m_nextRxSeq << ", buffsize=" << m_size);

  // Trim packet to fit Rx window specification
  if (headSeq < m_nextRxSeq) headSeq = m_nextRxSeq;
  if (m_data.size ())
    {
      SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> maxSeq = m_data.begin ()->first + SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> (m_maxBuffer);
      if (maxSeq < tailSeq) tailSeq = maxSeq;
      if (tailSeq < headSeq) headSeq = tailSeq;
    }
  // Remove overlapped bytes from packet
  BufIterator i = m_data.begin ();
  while (i != m_data.end () && i->first <= tailSeq)
    {
      SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> lastByteSeq = i->first + SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> (i->second->GetSize ());
      if (lastByteSeq > headSeq)
        {
          if (i->first > headSeq && lastByteSeq < tailSeq)
            { // Rare case: Existing packet is embedded fully in the new packet
              m_size -= i->second->GetSize ();
              m_data.erase (i++);
              continue;
            }
          if (i->first <= headSeq)
            { // Incoming head is overlapped
              headSeq = lastByteSeq;
            }
          if (lastByteSeq >= tailSeq)
            { // Incoming tail is overlapped
              tailSeq = i->first;
            }
        }
      ++i;
    }
  // We now know how much we are going to store, trim the packet
  if (headSeq >= tailSeq)
    {
      NS_LOG_LOGIC ("Nothing to buffer");
      return false; // Nothing to buffer anyway
    }
  else
    {
      uint32_t start = headSeq - prevHeadSeq;
      uint32_t length = tailSeq - headSeq;
      p = p->CreateFragment (start, length);
      NS_ASSERT (length == p->GetSize ());
    }
  // Insert packet into buffer
  NS_ASSERT (m_data.find (headSeq) == m_data.end ()); // Shouldn't be there yet
  m_data [ headSeq ] = p;
  NS_LOG_LOGIC ("Buffered packet of seqno=" << headSeq << " len=" << p->GetSize ());
  // Update variables
  m_size += p->GetSize ();      // Occupancy
  for (BufIterator i = m_data.begin (); i != m_data.end (); ++i)
    {
      if (i->first < m_nextRxSeq)
        {
          continue;
        }
      else if (i->first > m_nextRxSeq)
        {
          break;
        };
      m_nextRxSeq = i->first + SequenceNumber32 (i->second->GetSize ());
      m_availBytes += i->second->GetSize ();
    }
  NS_LOG_LOGIC ("Updated buffer occupancy=" << m_size << " nextRxSeq=" << m_nextRxSeq);
  if (m_gotFin && m_nextRxSeq == m_finSeq)
    { // Account for the FIN packet
      ++m_nextRxSeq;
    };
  return true;
}

template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
Ptr<Packet>
TcpRxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::Extract (uint32_t maxSize)
{
  NS_LOG_FUNCTION (this << maxSize);

  uint32_t extractSize = std::min (maxSize, m_availBytes);
  NS_LOG_LOGIC ("Requested to extract " << extractSize << " bytes from TcpRxBuffer of size=" << m_size);
  if (extractSize == 0) return 0;  // No contiguous block to return
  NS_ASSERT (m_data.size ()); // At least we have something to extract
  Ptr<Packet> outPkt = Create<Packet> (); // The packet that contains all the data to return
  BufIterator i;
  while (extractSize)
    { // Check the buffered data for delivery
      i = m_data.begin ();
      NS_ASSERT (i->first <= m_nextRxSeq); // in-sequence data expected
      // Check if we send the whole pkt or just a partial
      uint32_t pktSize = i->second->GetSize ();
      if (pktSize <= extractSize)
        { // Whole packet is extracted
          outPkt->AddAtEnd (i->second);
          m_data.erase (i);
          m_size -= pktSize;
          m_availBytes -= pktSize;
          extractSize -= pktSize;
        }
      else
        { // Partial is extracted and done
          outPkt->AddAtEnd (i->second->CreateFragment (0, extractSize));
          m_data[i->first + SequenceNumber32 (extractSize)] = i->second->CreateFragment (extractSize, pktSize - extractSize);
          m_data.erase (i);
          m_size -= extractSize;
          m_availBytes -= extractSize;
          extractSize = 0;
        }
    }
  if (outPkt->GetSize () == 0)
    {
      NS_LOG_LOGIC ("Nothing extracted.");
      return 0;
    }
  NS_LOG_LOGIC ("Extracted " << outPkt->GetSize ( ) << " bytes, bufsize=" << m_size
                             << ", num pkts in buffer=" << m_data.size ());
  return outPkt;
}

//Explicit instantiation of the types of TcpRxBuffers
template class TcpRxBuffer<uint32_t, int32_t>;
template class TcpRxBuffer<uint64_t, int64_t>;

} //namepsace ns3
