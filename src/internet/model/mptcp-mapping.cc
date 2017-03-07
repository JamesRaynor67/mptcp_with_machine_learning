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
#include <iostream>
#include <set>
#include <iterator>
#include <algorithm>
#include "ns3/mptcp-mapping.h"
#include "ns3/simulator.h"
#include "ns3/log.h"


NS_LOG_COMPONENT_DEFINE("MpTcpMapping");

namespace ns3
{
  
MpTcpMapping::MpTcpMapping() :
m_dataSequenceNumber(0),
m_subflowSequenceNumber(0),
m_dataLevelLength(0)
{
  NS_LOG_FUNCTION(this);
}

MpTcpMapping::MpTcpMapping (SequenceNumber64 dataSequence,
                            SequenceNumber32 subflowSequence,
                            uint16_t length) :
  m_dataSequenceNumber(dataSequence),
  m_subflowSequenceNumber(subflowSequence),
  m_dataLevelLength(length)
{
  NS_LOG_FUNCTION(this);
}

MpTcpMapping::~MpTcpMapping(void)
{
  NS_LOG_FUNCTION(this);
};


void
MpTcpMapping::SetMappingSize(uint16_t const& length)
{
  NS_LOG_DEBUG(this << " length=" << length);
  m_dataLevelLength = length;
}

SequenceNumber64
MpTcpMapping::GetDSNFromSSN(const SequenceNumber32& ssn) const
{
  NS_ASSERT(IsSSNInRange(ssn));
  return SequenceNumber64(ssn - HeadSSN()) + HeadDSN();
}

SequenceNumber32 MpTcpMapping::GetSSNFromDSN (const SequenceNumber64& dsn) const
{
  NS_ASSERT(IsDSNInRange(dsn));
  uint32_t length = uint32_t(dsn - HeadDSN());
  return SequenceNumber32(length) + HeadSSN();
}


std::ostream&
operator<<(std::ostream& os, const MpTcpMapping& mapping)
{
  os << "DSN [" << mapping.HeadDSN() << "-" << mapping.TailDSN ()
  << "] mapped to SSN [" <<  mapping.HeadSSN() << "-" <<  mapping.TailSSN() << "]";
  return os;
}

void
MpTcpMapping::SetHeadDSN(SequenceNumber64 const& dsn)
{
  NS_LOG_DEBUG(this << " headDSN=" << dsn);
  m_dataSequenceNumber = dsn;
}


void
MpTcpMapping::SetHeadSSN( SequenceNumber32 const& headSSN)
{
  NS_LOG_DEBUG(this << " mapping to ssn=" << headSSN);
  m_subflowSequenceNumber = headSSN;
}

bool
MpTcpMapping::operator==(const MpTcpMapping& mapping) const
{
  //!
  return (GetLength() == mapping.GetLength()
          && HeadDSN() == mapping.HeadDSN()
          && HeadSSN() == mapping.HeadSSN());
}

bool
MpTcpMapping::operator!=( const MpTcpMapping& mapping) const
{
  //!
  return !(*this == mapping);
}


SequenceNumber64
MpTcpMapping::HeadDSN() const
{
  return m_dataSequenceNumber;
}


SequenceNumber32
MpTcpMapping::HeadSSN() const
{
  return m_subflowSequenceNumber;
}

uint16_t
MpTcpMapping::GetLength() const
{
  return m_dataLevelLength;
}


SequenceNumber64
MpTcpMapping::TailDSN(void) const
{
  return(HeadDSN()+GetLength()-1);
}

SequenceNumber32
MpTcpMapping::TailSSN(void) const
{
  return(HeadSSN()+GetLength()-1);
}

bool
MpTcpMapping::operator<(MpTcpMapping const& m) const
{
  return (HeadSSN() < m.HeadSSN());
}


bool
MpTcpMapping::IsSSNInRange(SequenceNumber32 const& ssn) const
{
  return ( (HeadSSN() <= ssn) && (TailSSN() >= ssn) );
}

bool
MpTcpMapping::IsDSNInRange(SequenceNumber64 const& dsn) const
{
  return ( (HeadDSN() <= dsn) && (TailDSN() >= dsn) );
}


bool
MpTcpMapping::OverlapRangeSSN(const SequenceNumber32& headSSN, const uint16_t& len) const
{
  SequenceNumber32 tailSSN = headSSN + len-1;
  //!
  if( HeadSSN() >  tailSSN || TailSSN() < headSSN)
  {
    return false;
  }
  NS_LOG_DEBUG("SSN overlap");
  return true;
}

bool
MpTcpMapping::OverlapRangeDSN(const SequenceNumber64& headDSN, const uint16_t& len) const
{
  SequenceNumber64 tailDSN = headDSN + len-1;
  //!
  if( HeadDSN() >  tailDSN || TailDSN() < headDSN)
  {
    return false;
  }

  NS_LOG_DEBUG("DSN overlap");
  return true;
}

///////////////////////////////////////////////////////////
///// MpTcpMappingContainer
/////
MpTcpMappingContainer::MpTcpMappingContainer(void)
{
  NS_LOG_LOGIC(this);
}

MpTcpMappingContainer::~MpTcpMappingContainer(void)
{
  NS_LOG_LOGIC(this);
}

void
MpTcpMappingContainer::Dump() const
{
  NS_LOG_UNCOND("\n==== Dumping list of mappings ====");
  for(MappingSet::const_iterator it = m_mappings.begin(); it != m_mappings.end(); it++ )
  {
    NS_LOG_UNCOND( **it );
  }
  NS_LOG_UNCOND("==== End of dump ====\n");
}

Ptr<MpTcpMapping> MpTcpMappingContainer::AddMapping(const SequenceNumber64& dsn,
                                                    const SequenceNumber32& ssn,
                                                    uint16_t length)
{
  NS_LOG_LOGIC("Adding mapping");
  NS_ASSERT(length != 0);
  
  Ptr<MpTcpMapping> mapping = Create<MpTcpMapping>(dsn, ssn, length);
  
  pair<MappingSet::iterator,bool> res = m_mappings.insert(mapping);
  
  if(res.second){
    m_reverseMappings.insert(mapping);
    return mapping;
  }
  else
  {
    return nullptr;
  }
}

bool
MpTcpMappingContainer::FirstUnmappedSSN(SequenceNumber32& ssn) const
{
  //  NS_ASSERT(m_txBuffer);
  NS_LOG_FUNCTION_NOARGS();
  if(m_mappings.empty())
  {
    return false;
  }
  Ptr<MpTcpMapping> mapping = *(m_mappings.rbegin());
  ssn = mapping->TailSSN() + 1;
  return true;
}


bool
MpTcpMappingContainer::DiscardMapping(Ptr<MpTcpMapping> mapping)
{
  NS_LOG_LOGIC("discard mapping "<< mapping);
  bool erased = m_mappings.erase(mapping);
  m_reverseMappings.erase(mapping);
  return erased;
}

void MpTcpMappingContainer::DiscardMappingsInSSNRange(SequenceNumber32 ssn, uint32_t length)
{
  Ptr<MpTcpMapping> mapping = GetMappingForSSN(ssn);
  SequenceNumber32 currentSsn = ssn;
  uint32_t totalLength = 0;
  while(mapping && (totalLength < length))
  {
    if((currentSsn + (length - totalLength)) < mapping->TailSSN())
    {
      break;
    }
    DiscardMapping(mapping);
    totalLength += mapping->GetLength();
    currentSsn += mapping->GetLength();
    mapping = GetMappingForSSN(currentSsn);
  }
}

bool
MpTcpMappingContainer::GetMappingsStartingFromSSN(SequenceNumber32 ssn, set<Ptr<MpTcpMapping>>& missing)
{
  NS_LOG_FUNCTION(this << ssn );
  missing.clear();
  //  std::copy(it,m_mappings.end(),);
  //    http://www.cplusplus.com/reference/algorithm/equal_range/
  
  
  CompareMappingSsn comp;
  MappingSet::const_iterator it = lower_bound( m_mappings.begin(), m_mappings.end(), ssn, comp);
  
  copy(it, m_mappings.end(), inserter(missing, missing.begin()));
  return false;
}

Ptr<MpTcpMapping>
MpTcpMappingContainer::GetMappingForDSN(const SequenceNumber64& dsn) const
{
  NS_LOG_FUNCTION(dsn);
  if(m_mappings.empty())
  {
    return nullptr;
  }
  
  // Returns the first mapping that has a larger DSN
  // upper_bound returns the greater (using binary search)
  
  Ptr<MpTcpMapping> temp = Create<MpTcpMapping>();
  temp->SetHeadDSN(dsn);
  
  ReverseMappingSet::const_iterator it = m_reverseMappings.upper_bound(temp);
  
  if(it == m_reverseMappings.begin())
  {
    return nullptr;
  }
  
  it--;
  Ptr<MpTcpMapping> mapping = *it;
  NS_LOG_DEBUG("Is ssn in " << mapping << " ?");
  if (mapping->IsSSNInRange(dsn))
  {
    return mapping;
  }
  
  return nullptr;
}

Ptr<MpTcpMapping>
MpTcpMappingContainer::GetMappingForSSN(const SequenceNumber32& ssn) const
{
  NS_LOG_FUNCTION(ssn);
  if(m_mappings.empty())
  {
    return nullptr;
  }
  
  Ptr<MpTcpMapping> temp = Create<MpTcpMapping>();
  temp->SetHeadSSN(ssn);
  
  // Returns the first mapping that has a larger SSN
  // upper_bound returns the greater
  
  MappingSet::const_iterator it = m_mappings.upper_bound(temp);
  
  if(it == m_mappings.begin())
  {
    return nullptr;
  }
  
  it--;
  Ptr<MpTcpMapping> mapping = *it;
  NS_LOG_DEBUG("Is ssn in " << mapping << " ?");
  if (mapping->IsSSNInRange(ssn))
  {
    return mapping;
  }
  
  return nullptr;
  
}
  
} // namespace ns3