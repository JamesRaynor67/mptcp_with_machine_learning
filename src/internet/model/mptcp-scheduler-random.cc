#include <random>
#include <algorithm>
#include "ns3/mptcp-scheduler-random.h"
#include "ns3/mptcp-subflow.h"
#include "ns3/mptcp-meta-socket.h"
#include "ns3/log.h"
#include "ns3/random-variable-stream.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("MpTcpSchedulerRandom");
NS_OBJECT_ENSURE_REGISTERED(MpTcpSchedulerRandom);

TypeId
MpTcpSchedulerRandom::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MpTcpSchedulerRandom")
                              .SetParent<MpTcpScheduler> ()
                              .AddConstructor<MpTcpSchedulerRandom> ();
  return tid;
}


MpTcpSchedulerRandom::MpTcpSchedulerRandom() :  MpTcpScheduler()
{
  NS_LOG_FUNCTION(this);
  std::random_device rd;     // only used once to initialise (seed) engine
}

MpTcpSchedulerRandom::~MpTcpSchedulerRandom (void)
{
  NS_LOG_FUNCTION(this);
}

Ptr<MpTcpSubflow>
MpTcpSchedulerRandom::GetAvailableControlSubflow()
{
  NS_ASSERT(m_metaSock->GetNActiveSubflows() > 0);
  return  m_metaSock->GetActiveSubflow(0);
}

Ptr<MpTcpSubflow> MpTcpSchedulerRandom::GetAvailableSubflow (uint32_t dataToSend, uint32_t metaWindow)
{
  NS_LOG_FUNCTION(this);
  NS_ASSERT(m_metaSock);

  static Ptr<UniformRandomVariable> rdg = CreateObject<UniformRandomVariable> ();
  uint32_t subflowCount = m_metaSock->GetNActiveSubflows();
  vector<uint32_t> availableIndexRecord;

  Ptr<MpTcpSubflow> available = nullptr;

  for(uint32_t index = 0; index < subflowCount; ++index)
  {
    Ptr<MpTcpSubflow> subflow = m_metaSock->GetActiveSubflow(index);
    uint32_t subflowWindow = subflow->AvailableWindow();

    NS_LOG_DEBUG("subflow AvailableWindow  [" << subflowWindow << "]");
    uint32_t canSend = std::min(subflowWindow, metaWindow);

    //Check whether we can send (check silly window)
    if(canSend > 0 && subflow->CanSendPendingData(dataToSend))
    {
      availableIndexRecord.push_back(index);
    }
  }

  // ************ Random
  if(availableIndexRecord.size() > 0){
    // GetValue() gets the next random value, as a double in the specified range [min, max).
    available = m_metaSock->GetActiveSubflow(availableIndexRecord[size_t(rdg->GetValue(0, availableIndexRecord.size()))]);
    // std::uniform_int_distribution<std::mt19937::result_type> dist(0, availableIndexRecord.size()-1);
    // available = m_metaSock->GetActiveSubflow(availableIndexRecord[dist(this->rng)]);
  }

  // ***********  This is to force choose sublflow 1 once it's established *******************
  // NS_ASSERT(availableIndexRecord.size() <= 2); // assume at most two subflows
  // static bool subflow2FullyEstablished = false; // GetNSubflows
  //
  // if(availableIndexRecord.size() > 0 && availableIndexRecord.size() <= 2){
  //   if(m_metaSock->GetNSubflows() == 2){
  //     subflow2FullyEstablished = true;
  //   }
  //   if(subflow2FullyEstablished == false){
  //     NS_ASSERT(availableIndexRecord.size() <= 1);
  //     if(availableIndexRecord.size() == 1){
  //       available = m_metaSock->GetActiveSubflow(availableIndexRecord[0]);
  //     }
  //   }
  //   else{
  //     if(availableIndexRecord.size() == 2){
  //       available = m_metaSock->GetActiveSubflow(availableIndexRecord[0])->GetSubflowId() == 0?m_metaSock->GetActiveSubflow(availableIndexRecord[1]):m_metaSock->GetActiveSubflow(availableIndexRecord[0]);
  //     }
  //     else{
  //       if(availableIndexRecord.size() == 1 && m_metaSock->GetActiveSubflow(availableIndexRecord[0])->GetSubflowId() == 1){
  //         available = m_metaSock->GetActiveSubflow(availableIndexRecord[0]);
  //       }
  //     }
  //   }
  // }
  // ***********  Above is to force choose sublflow 1 once it's established *******************

  // ********** Force choose master flow **********
  // NS_ASSERT(availableIndexRecord.size() <= 2);
  // if(!availableIndexRecord.empty()){
  //   if(availableIndexRecord.size() == 2){
  //     // std::cout << "Hong Jiaming 60" << std::endl;
  //     available = m_metaSock->GetActiveSubflow(availableIndexRecord[0])->GetSubflowId() == 0?m_metaSock->GetActiveSubflow(availableIndexRecord[0]):m_metaSock->GetActiveSubflow(availableIndexRecord[1]);
  //   }
  //   else{
  //     if(availableIndexRecord.size() == 1 && m_metaSock->GetActiveSubflow(availableIndexRecord[0])->GetSubflowId() == 0){
  //       available = m_metaSock->GetActiveSubflow(availableIndexRecord[0]);
  //     }
  //   }
  // }

  return available;
}

// std::mt19937 MpTcpSchedulerRandom::rng(1); // set seed to be 1

} // namespace ns3
