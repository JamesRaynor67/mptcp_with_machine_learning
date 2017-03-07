#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <sstream>
#include <string>
#include <assert.h>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1   n2   n3   n4
//    point-to-point  |    |    |    |
//                    ================
//                      LAN 10.1.2.0

const int MAXHOSTNAME = 200;
const int MAXCONNECTIONS = 5;
const int MAXRECV = 500;

class RealSocket{
 public:
  RealSocket();
  virtual ~RealSocket();

  // Server initialization
  bool create();
  bool bind ( const int port );
  bool listen() const;
  bool accept ( RealSocket& ) const;

  // Client initialization
  bool connect ( const std::string host, const int port );

  // Data Transimission
  bool send ( const std::string ) const;
  int recv ( std::string& ) const;

  void set_non_blocking ( const bool );

  bool is_valid() const { return m_sock != -1; }

 private:

  int m_sock;
  sockaddr_in m_addr;
};

class SocketException{
 public:
  SocketException ( std::string s ) : m_s ( s ) {};
  ~SocketException (){};
  std::string description() { return m_s; }

 private:
  std::string m_s;
};

class ClientSocket : private RealSocket{
 public:
  ClientSocket ( std::string host, int port );
  virtual ~ClientSocket(){};

  const ClientSocket& operator << ( const std::string& ) const;
  const ClientSocket& operator >> ( std::string& ) const;
};

void SendIntegerToTensorflow(int number){
  try{
    ClientSocket client_socket("localhost", 12345);
    assert(number > 0);
    int count = 0;
    while(count < number){
      std::cout << count << std::endl;
      std::string reply;
        try{
          client_socket << static_cast<std::stringstream*>(&(std::stringstream() << count))->str();
          count++;
          client_socket >> reply;
        }
        catch(SocketException& ){
          std::cout << "We got an SocketException!" << std::endl;
        }
        std::cout << "We received this response from the server: \"" << reply << "\"\n";
    }
  }
  catch(SocketException& e){
    std::cout << "Exception was caught:" << e.description() << "\n";
  }
}

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SecondScriptExample");

int main (int argc, char *argv[]){
  NodeContainer nodes;
  nodes.Create (2); //creat 2 nodes they are p2p

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("2Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("20ms"));

  NetDeviceContainer devices;// put nodes in ndc
  devices = pointToPoint.Install (nodes);
  ////give them an address
  InternetStackHelper stack;
  stack.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("109.11.12.0", "255.255.255.0");//address setting

  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  //sink for reciever????
  PacketSinkHelper sink ("ns3::TcpSocketFactory",Address
                           (InetSocketAddress (Ipv4Address::GetAny (), 10)));
  //set a node as reciever
  ApplicationContainer app = sink.Install (nodes.Get(0));

  app.Start (Seconds (1.0));
  app.Stop (Seconds (10.0));

  OnOffHelper onOffHelper ("ns3::TcpSocketFactory", Address
                       (InetSocketAddress (Ipv4Address ("109.11.12.1"), 10)));
  onOffHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));

  onOffHelper.SetAttribute ("DataRate",StringValue ("2Mbps"));
  onOffHelper.SetAttribute ("PacketSize",UintegerValue(1280));
  // ApplicationContainer
  app    = onOffHelper.Install (nodes.Get (1));
  // Start the application
  app.Start (Seconds (1.0));
  app.Stop (Seconds (10.0));

  pointToPoint.EnablePcapAll ("testtcp");
  Simulator::Run ();

  Simulator::Schedule(Seconds(3.0), &SendIntegerToTensorflow, 3);
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

//////////////////////////////////////////////////////////////////////////
// Below is the implementation to communicate with tensorflow by Socket //
//////////////////////////////////////////////////////////////////////////

RealSocket::RealSocket():m_sock ( -1 ){
  memset(&m_addr, 0, sizeof(m_addr));
}

RealSocket::~RealSocket(){
  if(is_valid()){
    ::close(m_sock);
  }
}

bool RealSocket::create(){
  m_sock = socket (AF_INET, SOCK_STREAM, 0);
  if (!is_valid()){
    return false;
  }

  // TIME_WAIT - argh
  int on = 1;
  if(setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*) &on, sizeof(on)) == -1){
    return false;
  }
  return true;
}

bool RealSocket::bind(const int port){
  if(!is_valid()){
    return false;
  }

  m_addr.sin_family = AF_INET;
  m_addr.sin_addr.s_addr = INADDR_ANY;
  m_addr.sin_port = htons(port);

  int bind_return = ::bind(m_sock, (struct sockaddr *) &m_addr, sizeof(m_addr));

  if(bind_return == -1){
    return false;
  }

  return true;
}

bool RealSocket::listen() const{
  if(!is_valid()){
    return false;
  }

  int listen_return = ::listen(m_sock, MAXCONNECTIONS);

  if(listen_return == -1){
    return false;
  }

  return true;
}

bool RealSocket::accept(RealSocket& new_socket) const{
  int addr_length = sizeof(m_addr);
  new_socket.m_sock = ::accept ( m_sock, ( sockaddr * ) &m_addr, ( socklen_t * ) &addr_length );

  if(new_socket.m_sock <= 0){
    return false;
  }
  else{
    return true;
  }
}

bool RealSocket::send(const std::string s) const{
  int status = ::send ( m_sock, s.c_str(), s.size(), MSG_NOSIGNAL );
  if(status == -1){
    return false;
  }
  else{
    return true;
  }
}

int RealSocket::recv ( std::string& s ) const{
  char buf[MAXRECV + 1];

  s = "";

  memset(buf, 0, MAXRECV + 1);

  int status = ::recv(m_sock, buf, MAXRECV, 0);

  if(status == -1){
    std::cout << "status == -1   errno == " << errno << "  in Socket::recv\n";
    return 0;
  }
  else{
    if(status == 0 ){
      return 0;
    }
    else{
      s = buf;
      return status;
    }
  }
}

bool RealSocket::connect(const std::string host, const int port){
  if(!is_valid()){
    return false;
  }

  m_addr.sin_family = AF_INET;
  m_addr.sin_port = htons(port);

  int status = inet_pton(AF_INET, host.c_str(), &m_addr.sin_addr);

  if(errno == EAFNOSUPPORT){
    return false;
  }

  status = ::connect(m_sock, (sockaddr *) &m_addr, sizeof(m_addr));

  if(status == 0){
    return true;
  }
  else{
    return false;
  }
}

void RealSocket::set_non_blocking(const bool b){
  int opts;
  opts = fcntl( m_sock, F_GETFL);

  if(opts < 0){
    return;
  }

  if(b){
    opts = ( opts | O_NONBLOCK );
  }
  else{
    opts = ( opts & ~O_NONBLOCK );
  }
  fcntl (m_sock, F_SETFL, opts);
}

ClientSocket::ClientSocket ( std::string host, int port ){
  if ( ! RealSocket::create() ){
    throw SocketException ( "Could not create client socket." );
  }

  if ( ! RealSocket::connect ( host, port ) ){
    throw SocketException ( "Could not bind to port." );
  }
}

const ClientSocket& ClientSocket::operator << ( const std::string& s ) const{
  if ( ! RealSocket::send ( s ) ){
      throw SocketException ( "Could not write to socket." );
    }

  return *this;

}

const ClientSocket& ClientSocket::operator >> ( std::string& s ) const{
  if ( ! RealSocket::recv ( s ) ){
    throw SocketException ( "Could not read from socket." );
  }
  return *this;
}
