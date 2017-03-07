RealSocket// Implementation of the ClientSocket class

#include "rl_ClientSocket.h"
#include "rl_SocketException.h"

namespace ns3{

ClientSocket::ClientSocket ( std::string host, int port )
{
  if ( ! RealSocket::create() )
    {
      throw SocketException ( "Could not create client socket." );
    }

  if ( ! RealSocket::connect ( host, port ) )
    {
      throw SocketException ( "Could not bind to port." );
    }

}


const ClientSocket& ClientSocket::operator << ( const std::string& s ) const
{
  if ( ! RealSocket::send ( s ) )
    {
      throw SocketException ( "Could not write to socket." );
    }

  return *this;

}


const ClientSocket& ClientSocket::operator >> ( std::string& s ) const
{
  if ( ! RealSocket::recv ( s ) )
    {
      throw SocketException ( "Could not read from socket." );
    }

  return *this;
}

}
