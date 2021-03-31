#include "stateful_theta_router.hh"


bool StatefulThetaRouter::reset(Vertex source,
                                Vertex target,
                                num thetaValue)
{
  if(thetaValue <= lastValue)
  {
    if(source == lastSource and target == lastTarget)
    {
      lastValue = thetaValue;
      return false;
    }
  }

  lastValue = thetaValue;
  lastSource = source;
  lastTarget = target;

  doReset();

  return true;
}
