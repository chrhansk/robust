#include <gtest/gtest.h>

#include "robust_router_test.hh"

#include "log.hh"

#include "robust/robust_costs.hh"
#include "robust/simple_robust_router.hh"
#include "robust/searching_robust_router.hh"

#include "robust/active/goal_directed_active_router.hh"
#include "robust/active/bidirectional_active_router.hh"
#include "robust/active/simple_active_router.hh"

#include "robust/theta/simple_theta_router.hh"

RobustRouterTest::RobustRouterTest()
  : BasicTest()
{
  SimpleThetaRouter thetaRouter(graph,
                                costs,
                                deviations,
                                deviationSize);

  SimpleRobustRouter router(graph,
                            costs,
                            deviations,
                            deviationSize,
                            thetaRouter,
                            false);

  values = VertexMap<VertexMap<num>>(graph, VertexMap<num>());

  for(Vertex source : sources)
  {
    values(source) = VertexMap<num>(graph, (num) 0);

    for(Vertex target : targets)
    {
      RobustSearchResult result =
        router.shortestPath(source, target);

      values(source)(target) =
        (num) RobustCosts(costs,
                          deviations,
                          deviationSize).get(result.path);
    }
  }

}

void RobustRouterTest::testRobustRouter(RobustRouter& router) const
{
  idx calls = 0;
  idx queries = 0;

  for(Vertex source : sources)
  {
    for(Vertex target : targets)
    {
      ++queries;
      RobustSearchResult result = router.shortestPath(source, target);
      calls += result.calls;

      if(!result.found)
      {
        ASSERT_EQ(values(source)(target), inf);
      }
      else
      {
        ASSERT_TRUE(result.path.connects(source, target));
        ASSERT_EQ(values(source)(target),
                  RobustCosts(costs, deviations, deviationSize)
                  .get(result.path));
      }
    }
  }

  Log(info) << "Total number of calls for "
            << queries << " queries: "
            << calls;
}
