//
//  file-transfer-helper.h
//  ns3
//
//  Created by Lynne Salameh on 22/5/16.
//

#ifndef FileTransferHelper_H
#define FileTransferHelper_H

#include "ns3/object-factory.h"
#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include <string>

using namespace std;

namespace ns3
{
  
  class FileTransferHelper
  {
    
  public:
    FileTransferHelper(Address address);
    ~FileTransferHelper();
    
    void SetAttribute (string name, const AttributeValue& value);
    ApplicationContainer Install (NodeContainer nodes);
    
  private:
    ObjectFactory m_factory;
  };
}

#endif /* FileTransferHelper_H */
