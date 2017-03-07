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

#include "mptcp-scheduler.h"
#include "mptcp-meta-socket.h"
#include "mptcp-subflow.h"

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(MpTcpScheduler);
  
MpTcpScheduler::MpTcpScheduler () : m_metaSock(0)
{
}

MpTcpScheduler::~MpTcpScheduler()
{
}

void MpTcpScheduler::SetMeta(Ptr<MpTcpMetaSocket> metaSock)
{
  NS_ASSERT(metaSock);
  m_metaSock = metaSock;
}

uint32_t MpTcpScheduler::GetSendSizeForSubflow(Ptr<MpTcpSubflow> subflow, uint32_t segSize, uint32_t dataToSend)
{
  uint32_t subflowWindow = subflow->AvailableWindow();
  
  uint32_t length = std::min(subflowWindow, dataToSend);
  // For now we limit ourselves to a per packet length
  length = std::min(length, segSize);
  
  return length;
}
}
