#ifndef STATEFUL_THETA_ROUTER_HH
#define STATEFUL_THETA_ROUTER_HH

#include "graph/graph.hh"

#include "theta_router.hh"

class StatefulThetaRouter : public ThetaRouter
{
private:
  Vertex lastSource, lastTarget;
  num lastValue;

public:
  StatefulThetaRouter()
    : lastSource(-1),
      lastTarget(-1),
      lastValue(inf)
  {}

  virtual void doReset() = 0;

  bool reset(Vertex source, Vertex target, num thetaValue);
};


#endif /* STATEFUL_THETA_ROUTER_HH */
