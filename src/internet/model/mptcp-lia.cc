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

#include "ns3/log.h"
#include "mptcp-lia.h"
#include "mptcp-subflow.h"
#include "mptcp-meta-socket.h"

NS_LOG_COMPONENT_DEFINE ("MpTcpLia");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (MpTcpLia);

TypeId
MpTcpLia::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MpTcpLia")
    .SetParent<TcpNewReno> ()
    .SetGroupName ("Internet")
    .AddConstructor<MpTcpLia> ()
//    .AddTraceSource ("Alpha",
//                     "Value of the alp",
//                     MakeTraceSourceAccessor (&MpTcpLia::m_alpha),
//                     "ns3::WTH_IS_ZIS_TracedValueCallback"
//                     )
      ;
  ;
  return tid;
}

TypeId 
MpTcpLia::GetInstanceTypeId (void)
{
  return GetTypeId ();
}


// TODO we should aggregate CC for the mptcp case ?
MpTcpLia::MpTcpLia() : TcpNewReno()
                      , m_alpha (1)
{
  NS_LOG_FUNCTION_NOARGS();
}

MpTcpLia::~MpTcpLia ()
{
  NS_LOG_FUNCTION_NOARGS();
}


std::string
MpTcpLia::GetName () const
{
  return "MpTcpLia";
}

double
MpTcpLia::ComputeAlpha (Ptr<MpTcpMetaSocket> metaSock, Ptr<TcpSocketState> tcb) const
{
  // this method is called whenever a congestion happen in order to regulate the agressivety of m_subflows
  // m_alpha = cwnd_total * MAX(cwnd_i / rtt_i^2) / {SUM(cwnd_i / rtt_i))^2}   //RFC 6356 formula (2)
  
  NS_LOG_FUNCTION(this);
  
  double alpha = 0;
  double maxi = 0; // Matches the MAX(cwnd_i / rtt_i^2) part
  double sumi = 0; // SUM(cwnd_i / rtt_i)

  
  NS_ASSERT (metaSock);
  // TODO here
  for (uint32_t i = 0; i < metaSock->GetNActiveSubflows(); i++)
    {
      Ptr<MpTcpSubflow> sFlow = metaSock->GetActiveSubflow(i);

      Time time = sFlow->GetRttEstimator()->GetEstimate();
      double rtt = time.GetSeconds();
      double tmpi = tcb->m_cWnd.Get() / (rtt * rtt);
      
      if (maxi < tmpi)
        maxi = tmpi;

      sumi += tcb->m_cWnd.Get() / rtt;
    }
  alpha = (metaSock->GetTotalCwnd() * maxi) / (sumi * sumi);
  return alpha;
}

void
MpTcpLia::IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this);

  // Increase of cwnd based on current phase (slow start or congestion avoidance)
  if (tcb->m_cWnd < tcb->m_ssThresh)
    {
      tcb->m_cWnd += tcb->m_segmentSize;
      NS_LOG_INFO ("In SlowStart, updated tcb " << tcb << " cwnd to " << tcb->m_cWnd << " ssthresh " << tcb->m_ssThresh);
    }
  else
    {
      Ptr<MpTcpMetaSocket> metaSock = DynamicCast<MpTcpMetaSocket>(tcb->m_socket);
      uint32_t totalCwnd = metaSock->GetTotalCwnd ();

      m_alpha = ComputeAlpha (metaSock, tcb);
      double alpha_scale = 1;
//         The alpha_scale parameter denotes the precision we want for computing alpha
//                alpha  bytes_acked * MSS_i   bytes_acked * MSS_i
//          min ( --------------------------- , ------------------- )  (3)
//                 alpha_scale * cwnd_total              cwnd_i
    
    double adder = std::min (m_alpha* tcb->m_segmentSize * tcb->m_segmentSize / (totalCwnd* alpha_scale),
        static_cast<double>((tcb->m_segmentSize * tcb->m_segmentSize) / tcb->m_cWnd.Get ()));
      
    // Congestion avoidance mode, increase by (segSize*segSize)/cwnd. (RFC2581, sec.3.1)
      // To increase cwnd for one segSize per RTT, it should be (ackBytes*segSize)/cwnd

//    adder = std::max (1.0, adder);
      tcb->m_cWnd += static_cast<uint32_t> (adder);
      NS_LOG_INFO ("In CongAvoid, updated tcb " << tcb << " cwnd to " << tcb->m_cWnd << " ssthresh " << tcb->m_ssThresh);
    }
}

Ptr<TcpCongestionOps>
MpTcpLia::Fork ()
{
  return CreateObject<MpTcpLia>();
}

}