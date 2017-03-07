//
//  mp-onoff-application.cc
//  ns3
//
//  Created by Lynne Salameh on 21/7/16.
//

#include "mp-onoff-application.h"

#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/packet-socket-address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/random-variable-stream.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/mptcp-meta-socket.h"
#include "ns3/ipv4.h"

namespace ns3 {
  
  NS_LOG_COMPONENT_DEFINE ("MpOnOffApplication");
  
  NS_OBJECT_ENSURE_REGISTERED (MpOnOffApplication);
  
  TypeId
  MpOnOffApplication::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::MpOnOffApplication")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<MpOnOffApplication> ()
    .AddAttribute ("DataRate", "The data rate in on state.",
                   DataRateValue (DataRate ("500kb/s")),
                   MakeDataRateAccessor (&MpOnOffApplication::m_cbrRate),
                   MakeDataRateChecker ())
    .AddAttribute ("PacketSize", "The size of packets sent in on state",
                   UintegerValue (512),
                   MakeUintegerAccessor (&MpOnOffApplication::m_pktSize),
                   MakeUintegerChecker<uint32_t> (1))
    .AddAttribute ("Remote", "The address of the destination",
                   AddressValue (),
                   MakeAddressAccessor (&MpOnOffApplication::m_peer),
                   MakeAddressChecker ())
    .AddAttribute ("OnTime", "A RandomVariableStream used to pick the duration of the 'On' state.",
                   StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
                   MakePointerAccessor (&MpOnOffApplication::m_onTime),
                   MakePointerChecker <RandomVariableStream>())
    .AddAttribute ("OffTime", "A RandomVariableStream used to pick the duration of the 'Off' state.",
                   StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
                   MakePointerAccessor (&MpOnOffApplication::m_offTime),
                   MakePointerChecker <RandomVariableStream>())
    .AddAttribute ("MaxBytes",
                   "The total number of bytes to send. Once these bytes are sent, "
                   "no packet is sent again, even in on state. The value zero means "
                   "that there is no limit.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&MpOnOffApplication::m_maxBytes),
                   MakeUintegerChecker<uint64_t> ())
    .AddAttribute ("Protocol", "The type of protocol to use.",
                   TypeIdValue (UdpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&MpOnOffApplication::m_tid),
                   MakeTypeIdChecker ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&MpOnOffApplication::m_txTrace),
                     "ns3::Packet::TracedCallback")
    ;
    return tid;
  }
  
  
  MpOnOffApplication::MpOnOffApplication ()
  : m_socket (0),
  m_connected (false),
  m_residualBits (0),
  m_lastStartTime (Seconds (0)),
  m_totBytes (0),
  m_nextLocalAddress(0)
  {
    NS_LOG_FUNCTION (this);
  }
  
  MpOnOffApplication::~MpOnOffApplication()
  {
    NS_LOG_FUNCTION (this);
  }
  
  void
  MpOnOffApplication::SetMaxBytes (uint64_t maxBytes)
  {
    NS_LOG_FUNCTION (this << maxBytes);
    m_maxBytes = maxBytes;
  }
  
  Ptr<Socket>
  MpOnOffApplication::GetSocket (void) const
  {
    NS_LOG_FUNCTION (this);
    return m_socket;
  }
  
  int64_t
  MpOnOffApplication::AssignStreams (int64_t stream)
  {
    NS_LOG_FUNCTION (this << stream);
    m_onTime->SetStream (stream);
    m_offTime->SetStream (stream + 1);
    return 2;
  }
  
  void
  MpOnOffApplication::DoDispose (void)
  {
    NS_LOG_FUNCTION (this);
    
    m_socket = 0;
    // chain up
    Application::DoDispose ();
  }
  
  // Application Methods
  void MpOnOffApplication::StartApplication () // Called at time specified by Start
  {
    NS_LOG_FUNCTION (this);
    
    // Create the socket if not already
    if (!m_socket)
    {
      m_socket = Socket::CreateSocket (GetNode (), m_tid);
      if (Inet6SocketAddress::IsMatchingType (m_peer))
      {
        m_socket->Bind6 ();
      }
      else if (InetSocketAddress::IsMatchingType (m_peer) ||
               PacketSocketAddress::IsMatchingType (m_peer))
      {
        m_socket->Bind ();
      }
      
      m_socket->SetConnectCallback (MakeCallback (&MpOnOffApplication::ConnectionSucceeded, this),
                                    MakeCallback (&MpOnOffApplication::ConnectionFailed, this));
      
      m_socket->Connect (m_peer);
      m_socket->SetAllowBroadcast (true);
      m_socket->ShutdownRecv ();
      
      Ptr<MpTcpMetaSocket> meta = DynamicCast<MpTcpMetaSocket>(m_socket);
      if(meta)
      {
        //The initial interface bound to the subflow
        m_nextLocalAddress = 2;
        meta->SetFullyEstablishedCallback(MakeCallback(&MpOnOffApplication::ConnectionFullyEstablished, this));
      }
    }
    m_cbrRateFailSafe = m_cbrRate;
    
    // Insure no pending event
    CancelEvents ();
    // If we are not yet connected, there is nothing to do here
    // The ConnectionComplete upcall will start timers at that time
    if (!m_connected) return;
    ScheduleStartEvent ();
  }
  
  void MpOnOffApplication::StopApplication () // Called at time specified by Stop
  {
    NS_LOG_FUNCTION (this);
    
    CancelEvents ();
    if(m_socket != 0)
    {
      m_socket->Close ();
    }
    else
    {
      NS_LOG_WARN ("MpOnOffApplication found null socket to close in StopApplication");
    }
  }
  
  void MpOnOffApplication::CancelEvents ()
  {
    NS_LOG_FUNCTION (this);
    
    if (m_sendEvent.IsRunning () && m_cbrRateFailSafe == m_cbrRate )
    { // Cancel the pending send packet event
      // Calculate residual bits since last packet sent
      Time delta (Simulator::Now () - m_lastStartTime);
      int64x64_t bits = delta.To (Time::S) * m_cbrRate.GetBitRate ();
      m_residualBits += bits.GetHigh ();
    }
    m_cbrRateFailSafe = m_cbrRate;
    Simulator::Cancel (m_sendEvent);
    Simulator::Cancel (m_startStopEvent);
  }
  
  // Event handlers
  void MpOnOffApplication::StartSending ()
  {
    NS_LOG_FUNCTION (this);
    m_lastStartTime = Simulator::Now ();
    ScheduleNextTx ();  // Schedule the send packet event
    ScheduleStopEvent ();
  }
  
  void MpOnOffApplication::StopSending ()
  {
    NS_LOG_FUNCTION (this);
    CancelEvents ();
    
    ScheduleStartEvent ();
  }
  
  // Private helpers
  void MpOnOffApplication::ScheduleNextTx ()
  {
    NS_LOG_FUNCTION (this);
    
    if (m_maxBytes == 0 || m_totBytes < m_maxBytes)
    {
      uint32_t bits = m_pktSize * 8 - m_residualBits;
      NS_LOG_LOGIC ("bits = " << bits);
      Time nextTime (Seconds (bits /
                              static_cast<double>(m_cbrRate.GetBitRate ()))); // Time till next packet
      NS_LOG_LOGIC ("nextTime = " << nextTime);
      m_sendEvent = Simulator::Schedule (nextTime,
                                         &MpOnOffApplication::SendPacket, this);
    }
    else
    { // All done, cancel any pending events
      StopApplication ();
    }
  }
  
  void MpOnOffApplication::ScheduleStartEvent ()
  {  // Schedules the event to start sending data (switch to the "On" state)
    NS_LOG_FUNCTION (this);
    
    Time offInterval = Seconds (m_offTime->GetValue ());
    NS_LOG_LOGIC ("start at " << offInterval);
    m_startStopEvent = Simulator::Schedule (offInterval, &MpOnOffApplication::StartSending, this);
  }
  
  void MpOnOffApplication::ScheduleStopEvent ()
  {  // Schedules the event to stop sending data (switch to "Off" state)
    NS_LOG_FUNCTION (this);
    
    Time onInterval = Seconds (m_onTime->GetValue ());
    NS_LOG_LOGIC ("stop at " << onInterval);
    m_startStopEvent = Simulator::Schedule (onInterval, &MpOnOffApplication::StopSending, this);
  }
  
  
  void MpOnOffApplication::SendPacket ()
  {
    NS_LOG_FUNCTION (this);
    
    NS_ASSERT (m_sendEvent.IsExpired ());
    
    Ptr<Packet> packet = Create<Packet> (m_pktSize);
    m_txTrace (packet);
    m_socket->Send (packet);
    m_totBytes += m_pktSize;
    if (InetSocketAddress::IsMatchingType (m_peer))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                   << "s on-off application sent "
                   <<  packet->GetSize () << " bytes to "
                   << InetSocketAddress::ConvertFrom(m_peer).GetIpv4 ()
                   << " port " << InetSocketAddress::ConvertFrom (m_peer).GetPort ()
                   << " total Tx " << m_totBytes << " bytes");
    }
    else if (Inet6SocketAddress::IsMatchingType (m_peer))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                   << "s on-off application sent "
                   <<  packet->GetSize () << " bytes to "
                   << Inet6SocketAddress::ConvertFrom(m_peer).GetIpv6 ()
                   << " port " << Inet6SocketAddress::ConvertFrom (m_peer).GetPort ()
                   << " total Tx " << m_totBytes << " bytes");
    }
    m_lastStartTime = Simulator::Now ();
    m_residualBits = 0;
    ScheduleNextTx ();
  }
  
  
  void MpOnOffApplication::ConnectionSucceeded (Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION (this << socket);
    m_connected = true;
    
    if(!m_startStopEvent.IsRunning())
    {
      ScheduleStartEvent ();
    }
  }
  
  void MpOnOffApplication::ConnectionFailed (Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION (this << socket);
  }
  
  void MpOnOffApplication::ConnectionFullyEstablished(Ptr<MpTcpMetaSocket> socket)
  {
    //Get the node's ipv4 interfaces
    Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4>();
    NS_ASSERT(ipv4);
    
    if(m_nextLocalAddress < ipv4->GetNInterfaces())
    {
      //Try to create a new subflow
      Ipv4Address localAddress = ipv4->GetAddress(m_nextLocalAddress, 0).GetLocal();
      
      if(ipv4->IsUp (m_nextLocalAddress) &&
         ipv4->IsForwarding (m_nextLocalAddress))
      {
        
        m_nextLocalAddress++;
        
        InetSocketAddress localSocketAddr(localAddress, 0);
        socket->ConnectNewSubflow (localSocketAddr, m_peer);
      }
      
    }
  }
  
} // Namespace ns3
