// Definition of the ClientSocket class

#ifndef ClientSocket_class
#define ClientSocket_class

#include "rl_RealSocket.h"

namespace rl{

class ClientSocket : private RealSocket
{
 public:

  ClientSocket ( std::string host, int port );
  virtual ~ClientSocket(){};

  const ClientSocket& operator << ( const std::string& ) const;
  const ClientSocket& operator >> ( std::string& ) const;

};

}
#endif
