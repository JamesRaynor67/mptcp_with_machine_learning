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
#ifndef MPTCP_MAPPING_H
#define MPTCP_MAPPING_H

#include <stdint.h>
#include <vector>
#include <queue>
#include <list>
#include <set>
#include <map>
#include "ns3/object.h"
#include "ns3/simple-ref-count.h"
#include "ns3/sequence-number.h"

namespace ns3
{
  
  using namespace std;
  
  /**
   =====
   this should work with 64 bits seq number
   ======
   
   DSN=Data Sequence Number (mptcp option level)
   SSN=Subflow Sequence Number (TCP legacy seq nb)
   
   \todo DSN should be a uint64_t but that has implications over a lot of code,
   especially TCP buffers so it should be thought out with ns3 people beforehand
   */
  
  class MpTcpMapping : public SimpleRefCount<MpTcpMapping>
  {
  public:
    MpTcpMapping(void);
    
    MpTcpMapping(SequenceNumber64 dataSequence, SequenceNumber32 subflowSequence, uint16_t length);
    
    virtual ~MpTcpMapping(void);
    
    /**
     * \brief Set subflow sequence number
     * \param headSSN
     */
    void SetHeadSSN(SequenceNumber32 const& headSSN);
    
    /**
     *
     */
    void SetHeadDSN(SequenceNumber64 const& headDSN);
    
    /**
     * \brief Set mapping length
     */
    virtual void
    SetMappingSize(uint16_t const&);
    
    
    virtual bool
    OverlapRangeSSN(const SequenceNumber32& headSSN, const uint16_t& len) const;
    
    virtual bool
    OverlapRangeDSN(const SequenceNumber64& headDSN, const uint16_t& len) const;
    
    /**
     * \param ssn Data seqNb
     */
    bool IsSSNInRange(SequenceNumber32 const& ssn) const;
    
    /**
     * \param dsn Data seqNb
     */
    bool IsDSNInRange(SequenceNumber64 const& dsn) const;
    
    
    //If we are provided a DSN in our range, get the equivalent SSN
    //Error if DSN is not in range
    SequenceNumber32 GetSSNFromDSN (const SequenceNumber64& dsn) const;
    
    /**
     * \param ssn Subflow sequence number
     * \return the dsn for the mapping
     *
     */
    // TODO should be done by the user
    SequenceNumber64 GetDSNFromSSN(const SequenceNumber32& ssn) const;
    
    /**
     * \return The last MPTCP sequence number included in the mapping
     */
    SequenceNumber64 TailDSN (void) const;
    
    /**
     * \return The last subflow sequence number included in the mapping
     */
    SequenceNumber32 TailSSN (void) const;
    
    /**
     * Necessary for
     * std::set to sort mappings
     * Compares ssn
     * \brief Compares mapping based on their DSN number. It is required when inserting into a set
     */
    bool operator<(MpTcpMapping const& ) const;
    
    /**
     * \return MPTCP sequence number for the first mapped byte
     */
    virtual SequenceNumber64 HeadDSN() const;
    
    // TODO rename into GetMappedSSN Head ?
    /**
     * \return subflow sequence number for the first mapped byte
     */
    virtual SequenceNumber32 HeadSSN() const;
    
    /**
     * \return mapping length
     */
    virtual uint16_t GetLength() const ;
    
    /**
     * \brief Mapping are equal if everything concord, SSN/DSN and length
     */
    virtual bool operator==( const MpTcpMapping&) const;
    
    /**
     * \return Not ==
     */
    virtual bool operator!=( const MpTcpMapping& mapping) const;
    
    
    // TODO should be SequenceNumber64
  protected:
    
    SequenceNumber64 m_dataSequenceNumber;   //!< MPTCP sequence number
    SequenceNumber32 m_subflowSequenceNumber;  //!< subflow sequence number
    uint16_t m_dataLevelLength;  //!< mapping length / size
  };
  
  
  
  
  
  /**
   Depending on modifications allowed in upstream ns3, it may some day inherit from TcpTxbuffer etc ...
   Meanwhile we have a pointer towards the buffers.
   * \class MpTcpMappingContainer
   * Mapping handling
   Once a mapping has been advertised on a subflow, it must be honored. If the remote host already received the data
   (because it was sent in parallel over another subflow), then the received data must be discarded.
   
   For now, the map is a vector to allow for fast binary search using upper bound. We need a random access
   iterator container, otherwise upper_bound would be linear in size of set.
   
   TODO: it might be best to use a
   std::lower_bound on map
   Could be fun implemented as an interval tree
   http://www.geeksforgeeks.org/interval-tree/
   */
  class MpTcpMappingContainer
  {
  public:
    MpTcpMappingContainer(void);
    virtual ~MpTcpMappingContainer(void);
    
    
    void DiscardMappingsInSSNRange(SequenceNumber32 ssn, uint32_t length);
    
    /**
     * \param firstUnmappedSsn last mapped SSN.
     * \return true if non empty
     *
     */
    bool FirstUnmappedSSN(SequenceNumber32& firstUnmappedSsn) const;
    
    /**
     For debug purpose. Dump all registered mappings
     **/
    virtual void Dump() const;
    
    Ptr<MpTcpMapping> AddMapping(const SequenceNumber64& dsn, const SequenceNumber32& ssn, uint16_t length);
    
    /**
     * \param ssn The sequence number to look up
     * \returns The corresponding mapping, or nullptr if mapping is not found
     */
    Ptr<MpTcpMapping>
    GetMappingForSSN(const SequenceNumber32& ssn) const;
    
    Ptr<MpTcpMapping>
    GetMappingForDSN(const SequenceNumber64& dsn) const;
    
    /**
     * \param dsn
     */
    virtual bool GetMappingsStartingFromSSN(SequenceNumber32 ssn, set<Ptr<MpTcpMapping>>& mappings);
    
  protected:
    
    /**
     When Buffers work in non renegotiable mode,
     it should be possible to remove them one by one
     **/
    bool DiscardMapping(Ptr<MpTcpMapping> mapping);
    
    class CompareMappingSsn
    {
    public:
      bool operator()(const Ptr<MpTcpMapping> first, const Ptr<MpTcpMapping> second) const
      {
        return first->HeadSSN() < second->HeadSSN();
      }
      
      bool operator()(const SequenceNumber32& seq, const Ptr<MpTcpMapping> mapping)
      {
        return seq < mapping->HeadSSN();
      }
      bool operator()(const Ptr<MpTcpMapping> mapping, const SequenceNumber32& seq)
      {
        return mapping->HeadSSN() < seq;
      }
    };
    
    
    class CompareMappingDsn
    {
    public:
      bool operator()(const Ptr<MpTcpMapping> first, const Ptr<MpTcpMapping> second) const
      {
        return first->HeadDSN() < second->HeadDSN();
      }
      
      bool operator()(const SequenceNumber64& seq, const Ptr<MpTcpMapping> mapping)
      {
        return seq < mapping->HeadDSN();
      }
      bool operator()(const Ptr<MpTcpMapping> mapping, const SequenceNumber64& seq)
      {
        return mapping->HeadDSN() < seq;
      }
      
    };
    
    typedef set<Ptr<MpTcpMapping>, CompareMappingSsn> MappingSet;
    typedef set<Ptr<MpTcpMapping>, CompareMappingDsn> ReverseMappingSet;
    
    MappingSet m_mappings;     //!< it is a set ordered by SSN
    ReverseMappingSet m_reverseMappings; //!< the DSN mapped to SSN, ordered by DSN
    
  };
  
  /**
   This should be a set to prevent duplication and keep it ordered
   */
  
  std::ostream& operator<<(std::ostream &os, const MpTcpMapping& mapping);
  
  
  
} //namespace ns3
#endif //MP_TCP_TYPEDEFS_H