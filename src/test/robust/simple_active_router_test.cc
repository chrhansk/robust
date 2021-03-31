#include "robust_router_test.hh"

#include "robust/active/simple_active_router.hh"
#include "robust/theta/simple_theta_router.hh"

ADD_ROBUST_ROUTER_TEST(simple_active_router_test,
                       SimpleActiveRouter(graph,
                                          costs,
                                          deviations,
                                          deviationSize))
