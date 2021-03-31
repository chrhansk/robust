#include "robust_router_test.hh"

#include "robust/simple_robust_router.hh"
#include "robust/theta/simple_theta_router.hh"

ADD_ROBUST_ROUTER_TEST(bounding_router_test,
                       SimpleRobustRouter(graph,
                                          costs,
                                          deviations,
                                          deviationSize,
                                          thetaRouter,
                                          true))

