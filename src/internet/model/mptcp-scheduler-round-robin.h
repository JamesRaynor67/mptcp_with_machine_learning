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
#ifndef MPTCP_SCHEDULER_ROUND_ROBIN_H
#define MPTCP_SCHEDULER_ROUND_ROBIN_H

#include "ns3/mptcp-scheduler.h"

namespace ns3
{

class MpTcpSchedulerRoundRobin
//: public Object
: public MpTcpScheduler
{

public:
  static TypeId GetTypeId (void);

  MpTcpSchedulerRoundRobin();
  virtual ~MpTcpSchedulerRoundRobin ();
  
  /*
     Pick the next available subflow with space in its congestion window to send the next MSS on,
     in a round robin fashion from amongst active subflows.
   */
  virtual Ptr<MpTcpSubflow> GetAvailableSubflow (uint32_t dataToSend, uint32_t metaWindow) override;
  
  virtual Ptr<MpTcpSubflow> GetAvailableControlSubflow () override;

protected:
  uint32_t  m_lastUsedFlowId;        //!< keep track of last used subflow
};


} // end of 'ns3'

#endif /* MPTCP_SCHEDULER_ROUND_ROBIN_H */
