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
 *          Lynne Salameh <lsalameh@cs.ucl.ac.uk>
 */
#include "ns3/mptcp-scheduler-fastest-rtt.h"
#include "ns3/mptcp-subflow.h"
#include "ns3/mptcp-meta-socket.h"
#include "ns3/log.h"

namespace ns3
{
 
NS_LOG_COMPONENT_DEFINE("MpTcpSchedulerFastestRTT");
NS_OBJECT_ENSURE_REGISTERED(MpTcpSchedulerFastestRTT);
  
TypeId
MpTcpSchedulerFastestRTT::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MpTcpSchedulerFastestRTT")
                              .SetParent<MpTcpScheduler> ()
                              .AddConstructor<MpTcpSchedulerFastestRTT> ()
                      ;
  
  return tid;
}


MpTcpSchedulerFastestRTT::MpTcpSchedulerFastestRTT() :  MpTcpScheduler()
{
  NS_LOG_FUNCTION(this);
}

MpTcpSchedulerFastestRTT::~MpTcpSchedulerFastestRTT (void)
{
  NS_LOG_FUNCTION(this);
}

Ptr<MpTcpSubflow>
MpTcpSchedulerFastestRTT::GetAvailableControlSubflow()
{
  NS_ASSERT(m_metaSock->GetNActiveSubflows() > 0);
  return  m_metaSock->GetActiveSubflow(0);
}
  
Ptr<MpTcpSubflow> MpTcpSchedulerFastestRTT::GetAvailableSubflow (uint32_t dataToSend, uint32_t metaWindow)
{
  NS_LOG_FUNCTION(this);
  NS_ASSERT(m_metaSock);
  
  uint32_t subflowCount = m_metaSock->GetNActiveSubflows();
  Time lowestEstimate = Time::Max();
  
  Ptr<MpTcpSubflow> available = nullptr;
  
  for(uint32_t index = 0; index < subflowCount; ++index)
  {
    Ptr<MpTcpSubflow> subflow = m_metaSock->GetActiveSubflow(index);
    uint32_t subflowWindow = subflow->AvailableWindow();
    
    NS_LOG_DEBUG("subflow AvailableWindow  [" << subflowWindow << "]");
    uint32_t canSend = std::min(subflowWindow, metaWindow);
    //uint32_t canSend = subflowWindow;
    
    //Check whether we can send (check silly window)
    if(canSend > 0 && subflow->CanSendPendingData(dataToSend))
    {
      //Get the RTT estimate
      Time subflowRtt = subflow->GetRttEstimator()->GetEstimate ();
      if(subflowRtt < lowestEstimate)
      {
        lowestEstimate = subflowRtt;
        available = subflow;
      }
    }
  }
  
  return available;
}
  
} // namespace ns3

