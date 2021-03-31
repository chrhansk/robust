#include "robust_router_test.hh"

#include "robust/active/goal_directed_active_router.hh"
#include "robust/theta/simple_theta_router.hh"

ADD_ROBUST_ROUTER_TEST(bounding_robust_router_test,
                       GoalDirectedActiveRouter(graph,
                                                costs,
                                                deviations,
                                                deviationSize))
