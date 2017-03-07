//
//  file-transfer-application.hpp
//  ns3
//
//  Created by Lynne Salameh on 19/5/16.
//

#ifndef FileTransferApplication_H
#define FileTransferApplication_H

#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/mptcp-meta-socket.h"

namespace ns3
{
  class FileTransferApplication : public Application
  {
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId (void);
    
    FileTransferApplication ();
    
    virtual ~FileTransferApplication ();
    
    void SetMaxBytes (uint32_t maxBytes);
    Ptr<Socket> GetSocket (void) const;
    
    void StopApplicationExternal ();
    
  protected:
    virtual void DoDispose (void);
  private:
    // inherited from Application base class.
    virtual void StartApplication (void);    // Called at time specified by Start
    virtual void StopApplication (void);     // Called at time specified by Stop
    
    /**
     * \brief Send data until the L4 transmission buffer is full.
     */
    void SendData ();
    
    Ptr<Socket>     m_socket;       //!< Associated socket
    Address         m_peer;         //!< Peer address
    bool            m_connected;    //!< True if connected
    uint32_t        m_fileSize;     //!< Size of the file to send
    uint32_t        m_totalSent;     //!< Total bytes sent so far
    uint32_t        m_nextLocalAddress; //!<If we have multiple interfaces, points to the next free one
    TypeId          m_tid;          //!< Type of the socket used
    
  private:
    /**
     * \brief Connection Succeeded (called by Socket through a callback)
     * \param socket the connected socket
     */
    void ConnectionSucceeded (Ptr<Socket> socket);
    /**
     * \brief Connection Failed (called by Socket through a callback)
     * \param socket the connected socket
     */
    void ConnectionFailed (Ptr<Socket> socket);
    /**
     * \brief Send more data as soon as some has been transmitted.
     */
    void BytesAvailableCallback(Ptr<Socket> socket, uint32_t bytesAvailable); // for socket's SetSendCallback
    
    /*
     * Fires when a connection becomes a fully established MPTCP connection
     */
    void ConnectionFullyEstablished(Ptr<MpTcpMetaSocket> socket);

  };
}

#endif /* FileTransferApplication_H */
