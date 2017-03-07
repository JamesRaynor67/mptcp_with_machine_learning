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

#include <iostream>
#include <algorithm>
#include <cstring>

#include "ns3/packet.h"
#include "ns3/fatal-error.h"
#include "ns3/log.h"

#include "tcp-tx-buffer.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpTxBuffer");


//Define two different types of TcpTxBuffers
template<>
TypeId
TcpTxBuffer<uint32_t, int32_t>::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpTxBuffer32")
    .SetParent<Object> ()
    .SetGroupName ("Internet")
    .AddConstructor<TcpTxBuffer32> ()
    .AddTraceSource ("UnackSequence",
                     "First unacknowledged sequence number (SND.UNA)",
                     MakeTraceSourceAccessor (&TcpTxBuffer32::m_firstByteSeq),
                     "ns3::SequenceNumber32TracedValueCallback")
  ;
  return tid;
}
  
NS_OBJECT_ENSURE_REGISTERED (TcpTxBuffer32);
  
  template<>
  TypeId
  TcpTxBuffer<uint64_t, int64_t>::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::TcpTxBuffer64")
    .SetParent<Object> ()
    .SetGroupName ("Internet")
    .AddConstructor<TcpTxBuffer64> ()
    .AddTraceSource ("UnackSequence",
                     "First unacknowledged sequence number (SND.UNA)",
                     MakeTraceSourceAccessor (&TcpTxBuffer64::m_firstByteSeq),
                     "ns3::SequenceNumber64TracedValueCallback")
    ;
    return tid;
  }
  
NS_OBJECT_ENSURE_REGISTERED (TcpTxBuffer64);
  
/* A user is supposed to create a TcpSocket through a factory. In TcpSocket,
 * there are attributes SndBufSize and RcvBufSize to control the default Tx and
 * Rx window sizes respectively, with default of 128 KiByte. The attribute
 * SndBufSize is passed to TcpTxBuffer by TcpSocketBase::SetSndBufSize() and in
 * turn, TcpTxBuffer:SetMaxBufferSize(). Therefore, the m_maxBuffer value
 * initialized below is insignificant.
 */
  
template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
TcpTxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::TcpTxBuffer (NUMERIC_TYPE n)
  : m_firstByteSeq (n), m_size (0), m_maxBuffer (32768), m_data (0)
{
}

template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
TcpTxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::~TcpTxBuffer (void)
{
}

template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE>
TcpTxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::HeadSequence (void) const
{
  return m_firstByteSeq;
}

template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE>
TcpTxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::TailSequence (void) const
{
  return m_firstByteSeq + SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> (m_size);
}

template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
uint32_t
TcpTxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::Size (void) const
{
  return m_size;
}
  
template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
uint32_t
TcpTxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::MaxBufferSize (void) const
{
  return m_maxBuffer;
}

template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
void
TcpTxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::SetMaxBufferSize (uint32_t n)
{
  m_maxBuffer = n;
}

template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
uint32_t
TcpTxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::Available (void) const
{
  return m_maxBuffer - m_size;
}

template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
bool
TcpTxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::Add (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  NS_LOG_LOGIC ("Packet of size " << p->GetSize () << " appending to window starting at "
                                  << m_firstByteSeq << ", availSize="<< Available ());
  if (p->GetSize () <= Available ())
    {
      if (p->GetSize () > 0)
        {
          m_data.push_back (p);
          m_size += p->GetSize ();
          NS_LOG_LOGIC ("Updated size=" << m_size << ", lastSeq=" << m_firstByteSeq + SequenceNumber32 (m_size));
        }
      return true;
    }
  NS_LOG_LOGIC ("Rejected. Not enough room to buffer packet.");
  return false;
}

template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
uint32_t
TcpTxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::SizeFromSequence (const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE>& seq) const
{
  NS_LOG_FUNCTION (this << seq);
  // Sequence of last byte in buffer
  SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE> lastSeq = m_firstByteSeq + SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE>(m_size);
  // Non-negative size
  NS_LOG_LOGIC ("HeadSeq=" << m_firstByteSeq << ", lastSeq=" << lastSeq << ", size=" << m_size <<
                ", returns " << lastSeq - seq);
  return lastSeq - seq;
}

template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
Ptr<Packet>
TcpTxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::CopyFromSequence (uint32_t numBytes,
                                                          const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE>& seq)
{
  NS_LOG_FUNCTION (this << numBytes << seq);
  uint32_t s = std::min (numBytes, SizeFromSequence (seq)); // Real size to extract. Insure not beyond end of data
  if (s == 0)
    {
      return Create<Packet> (); // Empty packet returned
    }
  if (m_data.size () == 0)
    { // No actual data, just return dummy-data packet of correct size
      return Create<Packet> (s);
    }

  // Extract data from the buffer and return
  uint32_t offset = seq - m_firstByteSeq.Get ();
  uint32_t count = 0;      // Offset of the first byte of a packet in the buffer
  uint32_t pktSize = 0;
  bool beginFound = false;
  int pktCount = 0;
  Ptr<Packet> outPacket;
  NS_LOG_LOGIC ("There are " << m_data.size () << " number of packets in buffer");
  for (BufIterator i = m_data.begin (); i != m_data.end (); ++i)
    {
      pktCount++;
      pktSize = (*i)->GetSize ();
      if (!beginFound)
        { // Look for first fragment
          if (count + pktSize > offset)
            {
              NS_LOG_LOGIC ("First byte found in packet #" << pktCount << " at buffer offset " << count
                                                           << ", packet len=" << pktSize);
              beginFound = true;
              uint32_t packetOffset = offset - count;
              uint32_t fragmentLength = count + pktSize - offset;
              if (fragmentLength >= s)
                { // Data to be copied falls entirely in this packet
                  return (*i)->CreateFragment (packetOffset, s);
                }
              else
                { // This packet only fulfills part of the request
                  outPacket = (*i)->CreateFragment (packetOffset, fragmentLength);
                }
              NS_LOG_LOGIC ("Output packet is now of size " << outPacket->GetSize ());
            }
        }
      else if (count + pktSize >= offset + s)
        { // Last packet fragment found
          NS_LOG_LOGIC ("Last byte found in packet #" << pktCount << " at buffer offset " << count
                                                      << ", packet len=" << pktSize);
          uint32_t fragmentLength = offset + s - count;
          Ptr<Packet> endFragment = (*i)->CreateFragment (0, fragmentLength);
          outPacket->AddAtEnd (endFragment);
          NS_LOG_LOGIC ("Output packet is now of size " << outPacket->GetSize ());
          break;
        }
      else
        {
          NS_LOG_LOGIC ("Appending to output the packet #" << pktCount << " of offset " << count << " len=" << pktSize);
          outPacket->AddAtEnd (*i);
          NS_LOG_LOGIC ("Output packet is now of size " << outPacket->GetSize ());
        }
      count += pktSize;
    }
  NS_ASSERT (outPacket->GetSize () == s);
  return outPacket;
}

template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
void
TcpTxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::SetHeadSequence (const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE>& seq)
{
  NS_LOG_FUNCTION (this << seq);
  m_firstByteSeq = seq;
}

template<typename NUMERIC_TYPE, typename SIGNED_TYPE>
void
TcpTxBuffer<NUMERIC_TYPE, SIGNED_TYPE>::DiscardUpTo (const SequenceNumber<NUMERIC_TYPE, SIGNED_TYPE>& seq)
{
  NS_LOG_FUNCTION (this << seq);
  NS_LOG_LOGIC ("current data size=" << m_size << ", headSeq=" << m_firstByteSeq << ", maxBuffer=" << m_maxBuffer
                                     << ", numPkts=" << m_data.size ());
  // Cases do not need to scan the buffer
  if (m_firstByteSeq >= seq) return;

  // Scan the buffer and discard packets
  uint32_t offset = seq - m_firstByteSeq.Get ();  // Number of bytes to remove
  uint32_t pktSize;
  NS_LOG_LOGIC ("Offset=" << offset);
  BufIterator i = m_data.begin ();
  while (i != m_data.end ())
    {
      if (offset > (*i)->GetSize ())
        { // This packet is behind the seqnum. Remove this packet from the buffer
          pktSize = (*i)->GetSize ();
          m_size -= pktSize;
          offset -= pktSize;
          m_firstByteSeq += pktSize;
          i = m_data.erase (i);
          NS_LOG_LOGIC ("Removed one packet of size " << pktSize << ", offset=" << offset);
        }
      else if (offset > 0)
        { // Part of the packet is behind the seqnum. Fragment
          pktSize = (*i)->GetSize () - offset;
          *i = (*i)->CreateFragment (offset, pktSize);
          m_size -= offset;
          m_firstByteSeq += offset;
          NS_LOG_LOGIC ("Fragmented one packet by size " << offset << ", new size=" << pktSize);
          break;
        }
    }
  // Catching the case of ACKing a FIN
  if (m_size == 0)
    {
      m_firstByteSeq = seq;
    }
  NS_LOG_LOGIC ("size=" << m_size << " headSeq=" << m_firstByteSeq << " maxBuffer=" << m_maxBuffer
                        <<" numPkts="<< m_data.size ());
  NS_ASSERT (m_firstByteSeq == seq);
}
  
//Explicit instantiation of the TcpTxBuffer types
template class TcpTxBuffer<uint32_t, int32_t>;
template class TcpTxBuffer<uint64_t, int64_t>;

} // namepsace ns3
