#include "robust_router_test.hh"

#include "robust/searching_robust_router.hh"
#include "robust/theta/simple_theta_router.hh"

ADD_ROBUST_ROUTER_TEST(tightening_router_test,
                       SearchingRobustRouter(graph,
                                             costs,
                                             deviations,
                                             deviationSize,
                                             thetaRouter,
                                             {SearchingRobustRouter::BOUNDING, SearchingRobustRouter::TIGHTENING}))

