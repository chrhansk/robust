#include "robust_router_test.hh"

#include "robust/active/bidirectional_active_router.hh"
#include "robust/theta/simple_theta_router.hh"

ADD_ROBUST_ROUTER_TEST(bidirectional_active_router_test,
                       BidirectionalActiveRouter(graph,
                                                 costs,
                                                 deviations,
                                                 deviationSize))
