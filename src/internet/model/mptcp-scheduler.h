/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Author: Matthieu Coudron <matthieu.coudron@lip6.fr>
 */
#ifndef MPTCP_SCHEDULER_H
#define MPTCP_SCHEDULER_H

#include "ns3/object.h"

namespace ns3
{
  
using namespace std;

class MpTcpMetaSocket;
class MpTcpSubflow;

/**
 * This class is responsible for selecting the available subflow to schedule the next MSS on.
 *
 * The mptcp linux 0.90 scheduler is composed of mainly 2 functions:
 * -get_available_subflow
 * -get_next_segment
 *
 */
class MpTcpScheduler : public Object
{

public:

  MpTcpScheduler ();
  virtual ~MpTcpScheduler();

  virtual void SetMeta(Ptr<MpTcpMetaSocket> metaSock);
  
  uint32_t GetSendSizeForSubflow(Ptr<MpTcpSubflow> subflow, uint32_t segSize, uint32_t dataToSend);
  
  virtual Ptr<MpTcpSubflow> GetAvailableSubflow (uint32_t dataToSend, uint32_t metaWindow) = 0;
  
  //Get subflow to send empty control packets on, e.g. DATA_FIN.
  virtual Ptr<MpTcpSubflow> GetAvailableControlSubflow () = 0;
  
protected:
  Ptr<MpTcpMetaSocket> m_metaSock;  //!<A pointer back to the owning meta socket.

};


}


#endif /* MPTCP_SCHEDULER_H */
