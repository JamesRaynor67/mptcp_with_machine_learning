//
//  tcp-socket-wrapper.cpp
//  ns3
//
//  Created by Lynne Salameh on 17/7/16.
//  Copyright © 2016 University College London. All rights reserved.
//

#include "tcp-socket-wrapper.h"
#include "ns3/packet.h"
#include "ns3/node.h"

namespace ns3
{
  
  NS_OBJECT_ENSURE_REGISTERED(TcpSocketWrapper);
  
TcpSocketWrapper::TcpSocketWrapper () : Socket ()
{
  m_socket = nullptr;
}
  
TcpSocketWrapper::~TcpSocketWrapper ()
{
}
  
  TypeId TcpSocketWrapper::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::TcpSocketWrapper")
    .SetParent<TcpSocket> ()
    .AddConstructor<TcpSocketWrapper> ()
    .SetGroupName ("Internet")
    ;
    return tid;
  }
  
  TypeId TcpSocketWrapper::GetInstanceTypeId () const
  {
    return GetTypeId ();
  }
  
void TcpSocketWrapper::SetSocket (Ptr<TcpSocket> socket)
{
  
  if (m_socket){
    //Reassign the callbacks
    Callback<void, Ptr<Socket>, uint32_t > cbSend = m_socket->m_sendCb;
    Callback<void, Ptr<Socket> >  cbRcv = m_socket->m_receivedData;
    Callback<void, Ptr<Socket>, uint32_t>  cbDataSent = m_socket->m_dataSent;
    Callback<void, Ptr<Socket> >  cbConnectFail = m_socket->m_connectionFailed;
    Callback<void, Ptr<Socket> >  cbConnectSuccess = m_socket->m_connectionSucceeded;
    Callback<bool, Ptr<Socket>, const Address &> connectionRequest = m_socket->m_connectionRequest;
    Callback<void, Ptr<Socket>, const Address&> newConnectionCreated = m_socket->m_newConnectionCreated;
    
    socket->SetSendCallback(cbSend);
    socket->SetConnectCallback (cbConnectSuccess, cbConnectFail);   // Ok
    socket->SetDataSentCallback (cbDataSent);
    socket->SetRecvCallback (cbRcv);
    socket->SetAcceptCallback(connectionRequest, newConnectionCreated);
  }
  
  m_socket = socket;
  m_socket->SetSocketWrapper(this);
}
  
Ptr<TcpSocket> TcpSocketWrapper::GetSocket ()
{
  return m_socket;
}

enum Socket::SocketErrno TcpSocketWrapper::GetErrno (void) const
{
  return m_socket->GetErrno ();
}

enum Socket::SocketType TcpSocketWrapper::GetSocketType (void) const
{
  return m_socket->GetSocketType();
}

Ptr<Node>
TcpSocketWrapper::GetNode (void) const
{
  return m_socket->GetNode ();
}

void TcpSocketWrapper::SetConnectCallback (Callback<void, Ptr<Socket> > connectionSucceeded,
                                           Callback<void,  Ptr<Socket> > connectionFailed)
{
  m_socket->SetConnectCallback (connectionSucceeded, connectionFailed);
}

void TcpSocketWrapper::SetCloseCallbacks (Callback<void, Ptr<Socket> > normalClose,
                                          Callback<void, Ptr<Socket> > errorClose)
{
  m_socket->SetCloseCallbacks (normalClose, errorClose);
}

void TcpSocketWrapper::SetAcceptCallback (Callback<bool, Ptr<Socket>,
                                          const Address &> connectionRequest,
                                          Callback<void, Ptr<Socket>,
                                          const Address&> newConnectionCreated)
{
  m_socket->SetAcceptCallback (connectionRequest, newConnectionCreated);
}


void TcpSocketWrapper::SetDataSentCallback (Callback<void, Ptr<Socket>,
                                            uint32_t> dataSent)
{
  m_socket->SetDataSentCallback (dataSent);
}

void TcpSocketWrapper::SetSendCallback (Callback<void, Ptr<Socket>, uint32_t> sendCb)
{
  m_socket->SetSendCallback (sendCb);
}


void TcpSocketWrapper::SetRecvCallback (Callback<void, Ptr<Socket> > recvCb)
{
  m_socket->SetRecvCallback (recvCb);
}

int TcpSocketWrapper::Bind (const Address &address)
{
  return m_socket->Bind (address);
}



int TcpSocketWrapper::Bind ()
{
  return m_socket->Bind ();
}


int TcpSocketWrapper::Bind6 ()
{
  return m_socket->Bind6 ();
}


int TcpSocketWrapper::Close (void)
{
  return m_socket->Close ();
}


int TcpSocketWrapper::ShutdownSend (void)
{
  return m_socket->ShutdownSend ();
}


int TcpSocketWrapper::ShutdownRecv (void)
{
  return m_socket->ShutdownRecv ();
}


int TcpSocketWrapper::Connect (const Address &address)
{
  return m_socket->Connect (address);
}


int TcpSocketWrapper::Listen (void)
{
  return m_socket->Listen ();
}

uint32_t TcpSocketWrapper::GetTxAvailable (void) const
{
  return m_socket->GetTxAvailable ();
}


int TcpSocketWrapper::Send (Ptr<Packet> p, uint32_t flags)
{
  return m_socket->Send (p, flags);
}

int TcpSocketWrapper::SendTo (Ptr<Packet> p, uint32_t flags, const Address &toAddress)
{
  return m_socket->SendTo (p, flags, toAddress);
}


uint32_t TcpSocketWrapper::GetRxAvailable (void) const
{
  return m_socket->GetRxAvailable ();
}


Ptr<Packet> TcpSocketWrapper::Recv (uint32_t maxSize, uint32_t flags)
{
  return m_socket->Recv (maxSize, flags);
}


Ptr<Packet> TcpSocketWrapper::RecvFrom (uint32_t maxSize, uint32_t flags,
                                        Address &fromAddress)
{
  return m_socket->RecvFrom (maxSize, flags, fromAddress);
}


int TcpSocketWrapper::GetSockName (Address &address) const
{
  return m_socket->GetSockName (address);
}


int TcpSocketWrapper::GetPeerName (Address &address) const
{
  return m_socket->GetPeerName (address);
}


void TcpSocketWrapper::BindToNetDevice (Ptr<NetDevice> netdevice)
{
  m_socket->BindToNetDevice (netdevice);
}


Ptr<NetDevice> TcpSocketWrapper::GetBoundNetDevice ()
{
  return m_socket->GetBoundNetDevice ();
}

bool TcpSocketWrapper::SetAllowBroadcast (bool allowBroadcast)
{
  return m_socket->SetAllowBroadcast (allowBroadcast);
}


bool TcpSocketWrapper::GetAllowBroadcast () const
{
  return m_socket->GetAllowBroadcast ();
}

void TcpSocketWrapper::SetRecvPktInfo (bool flag)
{
  m_socket->SetRecvPktInfo(flag);
}


bool TcpSocketWrapper::IsRecvPktInfo () const
{
  return m_socket->IsRecvPktInfo ();
}

void TcpSocketWrapper::SetIpTos (uint8_t ipTos)
{
  m_socket->SetIpTos (ipTos);
}

uint8_t TcpSocketWrapper::GetIpTos (void) const
{
  return m_socket->GetIpTos ();
}

void TcpSocketWrapper::SetIpRecvTos (bool ipv4RecvTos)
{
  m_socket->SetIpRecvTos (ipv4RecvTos);
}


bool TcpSocketWrapper::IsIpRecvTos (void) const
{
  return m_socket->IsIpRecvTos ();
}

void TcpSocketWrapper::SetIpv6Tclass (int ipTclass)
{
  m_socket->SetIpv6Tclass (ipTclass);
}

uint8_t TcpSocketWrapper::GetIpv6Tclass (void) const
{
  return m_socket->GetIpv6Tclass ();
}

void TcpSocketWrapper::SetIpv6RecvTclass (bool ipv6RecvTclass)
{
  m_socket->SetIpv6RecvTclass (ipv6RecvTclass);
}


bool TcpSocketWrapper::IsIpv6RecvTclass (void) const
{
  return m_socket->IsIpv6RecvTclass ();
}

void TcpSocketWrapper::SetIpTtl (uint8_t ipTtl)
{
  m_socket->SetIpTtl (ipTtl);
}

uint8_t TcpSocketWrapper::GetIpTtl (void) const
{
  return m_socket->GetIpTtl ();
}

void TcpSocketWrapper::SetIpRecvTtl (bool ipv4RecvTtl)
{
  m_socket->SetIpRecvTtl (ipv4RecvTtl);
}


bool TcpSocketWrapper::IsIpRecvTtl (void) const
{
  return m_socket->IsIpRecvTtl ();
}

void TcpSocketWrapper::SetIpv6HopLimit (uint8_t ipHopLimit)
{
  m_socket->SetIpv6HopLimit (ipHopLimit);
}

uint8_t TcpSocketWrapper::GetIpv6HopLimit (void) const
{
  return m_socket->GetIpv6HopLimit ();
}


void TcpSocketWrapper::SetIpv6RecvHopLimit (bool ipv6RecvHopLimit)
{
  m_socket->SetIpv6RecvHopLimit (ipv6RecvHopLimit);
}


bool TcpSocketWrapper::IsIpv6RecvHopLimit (void) const
{
  return m_socket->IsIpv6RecvHopLimit ();
}


void TcpSocketWrapper::Ipv6JoinGroup (Ipv6Address address,
                                      Ipv6MulticastFilterMode filterMode,
                                      std::vector<Ipv6Address> sourceAddresses)
{
  m_socket->Ipv6JoinGroup (address, filterMode, sourceAddresses);
}

void TcpSocketWrapper::Ipv6JoinGroup (Ipv6Address address)
{
  m_socket->Ipv6JoinGroup (address);
}

void TcpSocketWrapper::Ipv6LeaveGroup (void)
{
  m_socket->Ipv6LeaveGroup ();
}

bool TcpSocketWrapper::IsManualIpv6Tclass (void) const
{
  return m_socket->IsManualIpv6Tclass ();
}

bool TcpSocketWrapper::IsManualIpTtl (void) const
{
  return m_socket->IsManualIpTtl ();
}

bool TcpSocketWrapper::IsManualIpv6HopLimit (void) const
{
  return m_socket->IsManualIpv6HopLimit ();
}
  
void TcpSocketWrapper::SetPriority (uint8_t priority)
{
  m_socket->SetPriority(priority);
}

uint8_t TcpSocketWrapper::GetPriority (void) const
{
  return m_socket->GetPriority();
}
  
}
