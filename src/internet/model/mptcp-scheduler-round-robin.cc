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
 *          Lynne Salameh <l.salameh@cs.ucl.ac.uk>
 */
#include "ns3/mptcp-scheduler-round-robin.h"
#include "ns3/mptcp-subflow.h"
#include "ns3/log.h"
#include "ns3/core-module.h"

// #include "ns3/rl-data-interface.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("MpTcpSchedulerRoundRobin");
NS_OBJECT_ENSURE_REGISTERED(MpTcpSchedulerRoundRobin);

TypeId
MpTcpSchedulerRoundRobin::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MpTcpSchedulerRoundRobin")
    .SetParent<MpTcpScheduler> ()
    //
    .AddConstructor<MpTcpSchedulerRoundRobin> ()
  ;
  return tid;
}


//Ptr<MptcpMetaSocket> metaSock
MpTcpSchedulerRoundRobin::MpTcpSchedulerRoundRobin() :
  MpTcpScheduler(),
  m_lastUsedFlowId(0)
{
  NS_LOG_FUNCTION(this);
}

MpTcpSchedulerRoundRobin::~MpTcpSchedulerRoundRobin (void)
{
  NS_LOG_FUNCTION(this);
}

Ptr<MpTcpSubflow> MpTcpSchedulerRoundRobin::GetAvailableSubflow (uint32_t dataToSend, uint32_t metaWindow)
{
  NS_LOG_FUNCTION(this);
  NS_ASSERT(m_metaSock);

  // uint32_t nbOfSubflows = m_metaSock->GetNActiveSubflows();
  // int index = 0;
  // // std::cout<< "Hong Jiaming: 1"<< endl;
  // static rl::InterfaceToRL rl_socket("127.0.0.1", 12345);
  // static uint32_t seq_num = 0;
  // rl_socket.add("ssn", seq_num);
  // seq_num++;
  // // std::cout<< "Hong Jiaming: 2"<< endl;
  // rl_socket.add("time", Simulator::Now().GetNanoSeconds());
  // rl_socket.add("nbOfSubflows", nbOfSubflows);
  // rl_socket.add("metaWindow", metaWindow);
  //
  // for(uint32_t index = 0; index < nbOfSubflows; index++){
  //   Ptr<MpTcpSubflow> subflow = m_metaSock->GetActiveSubflow(m_lastUsedFlowId);
  //   Ptr<TcpSocketState> tcb = subflow->GetTcb();
  //
  //   uint32_t subflowWindow = subflow->AvailableWindow();
  //   rl_socket.add("window"+std::to_string(index), subflowWindow);
  //   rl_socket.add("cWnd"+std::to_string(index), tcb->m_cWnd);
  //   rl_socket.add("lastAckedSeq"+std::to_string(index), tcb->m_lastAckedSeq.GetValue());
  //   rl_socket.add("highTxMark"+std::to_string(index), tcb->m_highTxMark.Get().GetValue());
  // }
  // // std::cout<< "Hong Jiaming: 3"<< endl;
  // rl_socket.send();
  // // std::cout<< "Hong Jiamindg: 4"<< endl;
  // std::string recv_str = rl_socket.recv();
  // // std::cout<< "Hong Jiamindg: 5"<< endl;
  // // std::cout<< "Choose: " << recv_str << " of " << nbOfSubflows << " subflows "<< endl;
  // int suggested_index = std::stoi(recv_str);
  //
  // if(suggested_index >= 0 && suggested_index < nbOfSubflows && (std::min(m_metaSock->GetActiveSubflow(suggested_index)->AvailableWindow(), metaWindow) > 0 && m_metaSock->GetActiveSubflow(suggested_index)->CanSendPendingData(dataToSend))){
  //   return m_metaSock->GetActiveSubflow(suggested_index);
  // }
  // else{
  //   return nullptr;
  // }

  int nbOfSubflows = m_metaSock->GetNActiveSubflows();
  int attempt = 0;

  NS_LOG_DEBUG ("Able to choose between [" << nbOfSubflows << "] subflows");

  while(attempt < nbOfSubflows)
  {
    attempt++;
    m_lastUsedFlowId = (m_lastUsedFlowId + 1) % nbOfSubflows;
    Ptr<MpTcpSubflow> subflow = m_metaSock->GetActiveSubflow(m_lastUsedFlowId);

    uint32_t subflowWindow = subflow->AvailableWindow();

    NS_LOG_DEBUG("subflow AvailableWindow  [" << subflowWindow << "]");

    uint32_t canSend = subflowWindow;

    //Check whether we can send (check silly window)
    if(canSend > 0 && subflow->CanSendPendingData(dataToSend))
    {
      // std::cout << "m_lastUsedFlowId" << m_lastUsedFlowId << endl;
      return subflow;
    }
  }
  // std::cout << "m_lastUsedFlowId is none!" << endl;
  NS_LOG_DEBUG("No subflow available");
  return nullptr;
}

Ptr<MpTcpSubflow> MpTcpSchedulerRoundRobin::GetAvailableControlSubflow ()
{
  NS_ASSERT(m_metaSock->GetNActiveSubflows() > 0 );
  return  m_metaSock->GetActiveSubflow(0);
}

} // end of 'ns3'
