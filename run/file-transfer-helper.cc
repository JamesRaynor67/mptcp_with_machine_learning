//
//  file-transfer-helper.cc
//  ns3
//
//  Created by Lynne Salameh on 22/5/16.
//

#include "file-transfer-helper.h"
#include "file-transfer-application.h"

namespace ns3
{
  FileTransferHelper::FileTransferHelper(Address address)
  {
    m_factory.SetTypeId("ns3::FileTransferApplication");
    m_factory.Set("Remote", AddressValue(address));
  }
  
  FileTransferHelper::~FileTransferHelper()
  {
  }
  
  void FileTransferHelper::SetAttribute (string name, const AttributeValue& value)
  {
    m_factory.Set(name, value);
  }
  
  ApplicationContainer FileTransferHelper::Install (NodeContainer nodes)
  {
    ApplicationContainer apps;
    for (NodeContainer::Iterator it = nodes.Begin (); it != nodes.End (); it++)
    {
      
      Ptr<FileTransferApplication> app = m_factory.Create<FileTransferApplication> ();
      
      (*it)->AddApplication (app);
      
      apps.Add (app);
    }
    
    return apps;
  }
  
}
