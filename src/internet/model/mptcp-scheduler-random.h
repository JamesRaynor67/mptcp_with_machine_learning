#ifndef MPTCP_SCHEDULER_RANDOM_H
#define MPTCP_SCHEDULER_RANDOM_H

#include <random>
#include "ns3/mptcp-scheduler.h"

namespace ns3
{

class MpTcpSchedulerRandom : public MpTcpScheduler
{

public:
  static TypeId GetTypeId (void);

  MpTcpSchedulerRandom();
  virtual ~MpTcpSchedulerRandom ();

  /**
   Return available subflow with lowest RTT
   */
  virtual Ptr<MpTcpSubflow> GetAvailableControlSubflow() override;

  virtual Ptr<MpTcpSubflow> GetAvailableSubflow (uint32_t dataToSend, uint32_t metaWindow) override;

protected:
  // static std::random_device rd; // seed for rng
  // static std::mt19937 rng;
};

} // end of 'ns3'

#endif /* MPTCP_SCHEDULER_RANDOM_H */
