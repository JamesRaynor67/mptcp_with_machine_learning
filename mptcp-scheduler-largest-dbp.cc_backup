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
#include "ns3/mptcp-scheduler-largest-dbp.h"
#include "ns3/mptcp-subflow.h"
#include "ns3/mptcp-meta-socket.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("MpTcpSchedulerLargestDBP");
NS_OBJECT_ENSURE_REGISTERED(MpTcpSchedulerLargestDBP);

TypeId
MpTcpSchedulerLargestDBP::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MpTcpSchedulerLargestDBP")
                              .SetParent<MpTcpScheduler> ()
                              .AddConstructor<MpTcpSchedulerLargestDBP> ()
                      ;

  return tid;
}


MpTcpSchedulerLargestDBP::MpTcpSchedulerLargestDBP() :  MpTcpScheduler()
{
  NS_LOG_FUNCTION(this);
}

MpTcpSchedulerLargestDBP::~MpTcpSchedulerLargestDBP (void)
{
  NS_LOG_FUNCTION(this);
}

Ptr<MpTcpSubflow>
MpTcpSchedulerLargestDBP::GetAvailableControlSubflow()
{
  NS_ASSERT(m_metaSock->GetNActiveSubflows() > 0);
  return  m_metaSock->GetActiveSubflow(0);
}

Ptr<MpTcpSubflow> MpTcpSchedulerLargestDBP::GetAvailableSubflow (uint32_t dataToSend, uint32_t metaWindow)
{
  NS_LOG_FUNCTION(this);
  NS_ASSERT(m_metaSock);

  uint32_t subflowCount = m_metaSock->GetNActiveSubflows();
  double largestDBP = 0;

  Ptr<MpTcpSubflow> available = nullptr;

  for(uint32_t index = 0; index < subflowCount; ++index)
  {
    Ptr<MpTcpSubflow> subflow = m_metaSock->GetActiveSubflow(index);
    uint32_t subflowWindow = subflow->AvailableWindow();

    NS_LOG_DEBUG("subflow AvailableWindow  [" << subflowWindow << "]");
    uint32_t canSend = std::min(subflowWindow, metaWindow);

    //Check whether we can send (check silly window)
    if(canSend > 0 && subflow->CanSendPendingData(dataToSend))
    {
      //Get the RTT estimate
      double delayBandwidthProduct = subflow->GetDelayBandwidthProduct();
      if(delayBandwidthProduct > largestDBP)
      {
        largestDBP = delayBandwidthProduct;
        available = subflow;
      }
    }
  }
  // std::cout << "Hong Jiaming 35: largestDBP == " << largestDBP << std::endl;
  return available;
}

} // namespace ns3
