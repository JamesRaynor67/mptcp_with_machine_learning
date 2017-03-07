//
//  file-transfer-application.cpp
//  ns3
//
//  Created by Lynne Salameh on 19/5/16.
//

#include "file-transfer-application.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/ipv4.h"

using namespace std;

namespace ns3
{
  NS_OBJECT_ENSURE_REGISTERED(FileTransferApplication);
  
  FileTransferApplication::FileTransferApplication () : m_socket (nullptr),
  m_connected (false),
  m_totalSent (0),
  m_nextLocalAddress (0)
  {
    
  }
  
  FileTransferApplication::~FileTransferApplication ()
  {
  }
  
  TypeId
  FileTransferApplication::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::FileTransferApplication")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<FileTransferApplication> ()
    .AddAttribute ("FileSize", "The size of the file to send.",
                   UintegerValue (1e9),
                   MakeUintegerAccessor (&FileTransferApplication::m_fileSize),
                   MakeUintegerChecker<uint32_t> (1))
    .AddAttribute ("Remote", "The address of the destination",
                   AddressValue (),
                   MakeAddressAccessor (&FileTransferApplication::m_peer),
                   MakeAddressChecker ())
    .AddAttribute ("Protocol", "The type of protocol to use.",
                   TypeIdValue (TcpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&FileTransferApplication::m_tid),
                   MakeTypeIdChecker ())
    ;
    return tid;
  }
  
  Ptr<Socket>
  FileTransferApplication::GetSocket (void) const
  {
    return m_socket;
  }
  
  void FileTransferApplication::DoDispose (void)
  {
    m_socket = nullptr;
    // chain up
    Application::DoDispose ();
  }
  
  // Application Methods
  void FileTransferApplication::StartApplication (void) // Called at time specified by Start
  {
    // Create the socket if not already
    if (!m_socket)
    {
      //First get the type of socket
      m_socket = Socket::CreateSocket (GetNode (), m_tid);
      
      if (Inet6SocketAddress::IsMatchingType (m_peer))
      {
        m_socket->Bind6 ();
      }
      else if (InetSocketAddress::IsMatchingType (m_peer))
      {
        m_socket->Bind ();
      }
      
      m_nextLocalAddress = 2;
      
      m_socket->SetConnectCallback (MakeCallback (&FileTransferApplication::ConnectionSucceeded, this),
                                    MakeCallback (&FileTransferApplication::ConnectionFailed, this));
      m_socket->SetSendCallback (MakeCallback (&FileTransferApplication::BytesAvailableCallback, this));
      
      Ptr<MpTcpMetaSocket> meta = DynamicCast<MpTcpMetaSocket>(m_socket);
      if(meta)
      {
        meta->SetFullyEstablishedCallback(MakeCallback(&FileTransferApplication::ConnectionFullyEstablished, this));
      }
      
      m_socket->Connect (m_peer);
      m_socket->ShutdownRecv ();
      
    }
    if (m_connected)
    {
      SendData ();
    }
  }
  
  void FileTransferApplication::StopApplication (void) // Called at time specified by Stop
  {
    if (m_socket != nullptr)
    {
      m_socket->Close ();
      m_connected = false;
    }
  }
  
  // Private helpers
  
  void FileTransferApplication::SendData (void)
  {
    
    uint32_t bytes = m_socket->GetTxAvailable();
    
    uint32_t size = m_fileSize - m_totalSent;
    uint32_t sizeToSend = min(size, bytes);
    
    NS_ASSERT(sizeToSend > 0);
    
    Ptr<Packet> aPacket = Create<Packet>(sizeToSend);
    
    int actual = m_socket->Send (aPacket);
    if (actual > 0)
    {
      m_totalSent += actual;
    }
    
    if (m_totalSent == m_fileSize)
    {
      m_connected = false;
      m_socket->Close();
    }
    
  }
  
  void FileTransferApplication::ConnectionSucceeded (Ptr<Socket> socket)
  {
    m_connected = true;
    SendData ();
  }
  
  void FileTransferApplication::ConnectionFailed (Ptr<Socket> socket)
  {
    
  }
  
  void FileTransferApplication::BytesAvailableCallback (Ptr<Socket> socket, uint32_t bytesAvailable)
  {
    if (m_connected)
    { // Only send new data if the connection has completed
      SendData ();
    }
  }
  
  void FileTransferApplication::ConnectionFullyEstablished(Ptr<MpTcpMetaSocket> socket)
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
  
  void FileTransferApplication::StopApplicationExternal ()
  {
    m_stopEvent.Cancel();
    m_stopEvent = Simulator::ScheduleNow(&FileTransferApplication::StopApplication, this);
  }
}
