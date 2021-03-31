#include "basic_test.hh"

#include <vector>
#include <random>

#include "graph/graph.hh"
#include "graph/edge_map.hh"
#include "graph/vertex_map.hh"

#include "arcflags/metis_partition.hh"

#include "robust/arcflags/bounded_arcflags.hh"
#include "robust/arcflags/extended_arcflags.hh"
#include "robust/arcflags/simple_arcflags.hh"

#include "robust/arcflags/fast_arcflag_preprocessor.hh"

#include "robust/arcflags/robust_arcflag_preprocessor.hh"
#include "robust/arcflags/arcflag_theta_router.hh"

#include "robust/contraction/parallel_robust_contraction_preprocessor.hh"
#include "robust/contraction/robust_contraction_hierarchy.hh"
#include "robust/contraction/robust_contraction_preprocessor.hh"

#include "robust/robust_costs.hh"
#include "robust/robust_utils.hh"
#include "robust/simple_robust_router.hh"

#include "robust/theta/bidirectional_bounding_router.hh"
#include "robust/theta/bidirectional_goal_directed_router.hh"
#include "robust/theta/bounding_router.hh"
#include "robust/theta/goal_directed_router.hh"
#include "robust/theta/goal_directed_bounding_router.hh"
#include "robust/theta/simple_theta_router.hh"
#include "robust/theta/theta_router.hh"

const int deviationSize = 5;
const int numValues = 10;

class ThetaRouterTest : public BasicTest
{
protected:
  void testThetaRouter(ThetaRouter& thetaRouter) const;

  METISPartition partition;
  VertexMap<VertexMap<num>> values;

public:
  ThetaRouterTest();
};


ThetaRouterTest::ThetaRouterTest()
  : BasicTest(),
    partition(graph, 32)
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

      assert(result.found);

      values(source)(target) =
        (num) RobustCosts(costs,
                          deviations,
                          deviationSize).get(result.path);
    }
  }
}

void ThetaRouterTest::testThetaRouter(ThetaRouter& thetaRouter) const
{
  SimpleRobustRouter router(graph,
                            costs,
                            deviations,
                            deviationSize,
                            thetaRouter,
                            false);

  RobustCosts robustCosts(costs, deviations, deviationSize);

  for(Vertex source : sources)
  {
    for(Vertex target : targets)
    {
      const num expectedValue = values(source)(target);

      RobustSearchResult result = router.shortestPath(source, target);

      ASSERT_TRUE(result.found);
      ASSERT_EQ(expectedValue,
                robustCosts.get(result.path));
    }
  }
}

TEST_F(ThetaRouterTest, testGoalDirectedRouter)
{
  GoalDirectedRouter router(graph,
                            costs,
                            deviations,
                            deviationSize);

  testThetaRouter(router);
}

TEST_F(ThetaRouterTest, testGoalDirectedBoundingRouter)
{
  GoalDirectedBoundingRouter router(graph,
                                    costs,
                                    deviations,
                                    deviationSize);

  testThetaRouter(router);
}

TEST_F(ThetaRouterTest, testBidirectionalGoalDirectedAverage)
{
  BidirectionalGoalDirectedRouter router(graph,
                                         costs,
                                         deviations,
                                         deviationSize,
                                         BidirectionalGoalDirectedRouter::Average);

  testThetaRouter(router);
}

TEST_F(ThetaRouterTest, testBidirectionalGoalDirectedMax)
{
  BidirectionalGoalDirectedRouter router(graph,
                                         costs,
                                         deviations,
                                         deviationSize,
                                         BidirectionalGoalDirectedRouter::Max);

  testThetaRouter(router);
}

TEST_F(ThetaRouterTest, testBidirectionalGoalDirectedMin)
{
  BidirectionalGoalDirectedRouter router(graph,
                                         costs,
                                         deviations,
                                         deviationSize,
                                         BidirectionalGoalDirectedRouter::Min);

  testThetaRouter(router);
}

TEST_F(ThetaRouterTest, testBidirectionalGoalDirectedRecomputation)
{
  BidirectionalGoalDirectedRouter router(graph,
                                         costs,
                                         deviations,
                                         deviationSize);

  router.setExtendedRecomputation(true);

  testThetaRouter(router);
}

TEST_F(ThetaRouterTest, testBoundingRouter)
{
  BoundingRouter router(graph,
                        costs,
                        deviations,
                        deviationSize);

  testThetaRouter(router);
}

TEST_F(ThetaRouterTest, testBidirectionalBoundingRouter)
{
  BidirectionalBoundingRouter router(graph,
                                     costs,
                                     deviations,
                                     deviationSize);

  testThetaRouter(router);
}


TEST_F(ThetaRouterTest, testSimpleArcFlagRouter)
{
  RobustArcFlagPreprocessor preprocessor(graph,
                                         costs,
                                         deviations,
                                         partition);

  Bidirected<SimpleArcFlags> flags(graph, partition);

  preprocessor.computeFlags(flags, false);

  ArcFlagThetaRouter<SimpleArcFlags> router(graph,
                                            costs,
                                            deviations,
                                            partition,
                                            flags);

  testThetaRouter(router);
}

TEST_F(ThetaRouterTest, testBoundedArcFlagRouter)
{
  RobustArcFlagPreprocessor preprocessor(graph,
                                         costs,
                                         deviations,
                                         partition);

  Bidirected<BoundedArcFlags> flags(graph, partition);

  preprocessor.computeFlags(flags, false);

  ArcFlagThetaRouter<BoundedArcFlags> router(graph,
                                             costs,
                                             deviations,
                                             partition,
                                             flags);

  testThetaRouter(router);
}

TEST_F(ThetaRouterTest, testExtendedArcFlagRouter)
{
  RobustArcFlagPreprocessor preprocessor(graph,
                                         costs,
                                         deviations,
                                         partition);

  Bidirected<ExtendedArcFlags> flags(graph, partition);

  preprocessor.computeFlags(flags, false);

  ArcFlagThetaRouter<ExtendedArcFlags> router(graph,
                                              costs,
                                              deviations,
                                              partition,
                                              flags);

  testThetaRouter(router);
}

TEST_F(ThetaRouterTest, testFastArcFlagRouter)
{
  FastArcFlagPreprocessor preprocessor(graph,
                                       costs,
                                       deviations,
                                       partition);

  Bidirected<ExtendedArcFlags> flags(graph, partition);

  preprocessor.computeFlags(flags, 16, false);

  ArcFlagThetaRouter<ExtendedArcFlags> router(graph,
                                               costs,
                                               deviations,
                                               partition,
                                               flags);

  testThetaRouter(router);
}

TEST_F(ThetaRouterTest, testParallelFastArcFlagRouter)
{
  FastArcFlagPreprocessor preprocessor(graph,
                                       costs,
                                       deviations,
                                       partition);

  Bidirected<ExtendedArcFlags> flags(graph, partition);

  preprocessor.computeFlags(flags, 16, true);

  ArcFlagThetaRouter<ExtendedArcFlags> router(graph,
                                               costs,
                                               deviations,
                                               partition,
                                               flags);

  testThetaRouter(router);
}

TEST_F(ThetaRouterTest, testContractionHierarchy)
{
  RobustContractionPreprocessor preprocessor(graph,
                                             costs,
                                             deviations);

  RobustContractionHierarchy hierarchy(preprocessor.computeHierarchy());

  auto contractionRouter = hierarchy.getRouter();

  testThetaRouter(contractionRouter);
}

TEST_F(ThetaRouterTest, testParallelContractionHierarchy)
{
  ParallelRobustContractionPreprocessor preprocessor(graph,
                                                     costs,
                                                     deviations);

  RobustContractionHierarchy hierarchy(preprocessor.computeHierarchy());

  auto contractionRouter = hierarchy.getRouter();

  testThetaRouter(contractionRouter);
}
