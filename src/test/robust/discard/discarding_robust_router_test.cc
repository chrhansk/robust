#include "robust/robust_router_test.hh"

#include "arcflags/metis_partition.hh"

#include "robust/discard/discarding_preprocessor.hh"
#include "robust/discard/discarding_robust_router.hh"
#include "robust/theta/simple_theta_router.hh"

class DiscardingRobustRouterTest : public RobustRouterTest
{
protected:
  METISPartition partition;
  DiscardingPreprocessor preprocessor;

public:
  DiscardingRobustRouterTest();
};

DiscardingRobustRouterTest::DiscardingRobustRouterTest()
  : RobustRouterTest(),
    partition(graph, 64),
    preprocessor(graph, costs, deviations, partition)
{
}

TEST_F(DiscardingRobustRouterTest, Test_discarding_robust_router_test)
{
  SimpleThetaRouter thetaRouter(graph,
                                costs,
                                deviations,
                                deviationSize);

  DiscardingRobustRouter router(graph,
                                costs,
                                deviations,
                                deviationSize,
                                thetaRouter,
                                preprocessor,
                                SearchingRobustRouter::TIGHTENING);
  testRobustRouter(router);
}

/*
ADD_ROBUST_ROUTER_TEST(searching_router_test,
                       SearchingRobustRouter(graph,
                                             costs,
                                             deviations,
                                             deviationSize,
                                             thetaRouter,
                                             SearchingRobustRouter::BOUNDING))
*/
