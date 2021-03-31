#include "basic_test.hh"

#include "log.hh"

#include "arcflags/metis_partition.hh"

#include "robust/arcflags/robust_arcflag_preprocessor.hh"
#include "robust/arcflags/simple_arcflags.hh"
#include "robust/arcflags/arcflag_theta_router.hh"
#include "robust/theta/simple_theta_router.hh"

class RobustArcFlagPreprocessorTest : public BasicTest
{
protected:
  METISPartition partition;
  VertexMap<VertexMap<num>> values;

  void testRouter(ThetaRouter& router,
                  const ValueVector& values);

public:
  RobustArcFlagPreprocessorTest();
};


RobustArcFlagPreprocessorTest::RobustArcFlagPreprocessorTest()
  : BasicTest(),
    partition(graph, 32)
{
}

void RobustArcFlagPreprocessorTest::testRouter(ThetaRouter& router,
                                               const ValueVector& values)
{
  SimpleThetaRouter simpleRouter(graph, costs, deviations, deviationSize);

  for(const Vertex& source : sources)
  {
    for(const Vertex& target : targets)
    {
      for(const num& value : values)
      {
        auto result = router.shortestPath(source,
                                          target,
                                          value);

        auto simpleResult = simpleRouter.shortestPath(source,
                                                      target,
                                                      value);

        ASSERT_EQ(result.cost, simpleResult.cost);
      }
    }
  }
}

TEST_F(RobustArcFlagPreprocessorTest, testAllValues)
{
  RobustArcFlagPreprocessor preprocessor(graph,
                                         costs,
                                         deviations,
                                         partition);

  Bidirected<SimpleArcFlags> flags(graph, partition);

  preprocessor.computeFlags(flags, true);

  ValueVector values = thetaValues(graph, deviations);

  ArcFlagThetaRouter<SimpleArcFlags> arcFlagRouter(graph,
                                                   costs,
                                                   deviations,
                                                   partition,
                                                   flags);

  testRouter(arcFlagRouter, values);
}

TEST_F(RobustArcFlagPreprocessorTest, testSelectedValues)
{
  ValueVector values = {25, 15};

  RobustArcFlagPreprocessor preprocessor(graph,
                                         costs,
                                         deviations,
                                         partition,
                                         values);

  Bidirected<SimpleArcFlags> flags(graph, partition);

  preprocessor.computeFlags(flags, false);

  ArcFlagThetaRouter<SimpleArcFlags> arcFlagRouter(graph,
                                                   costs,
                                                   deviations,
                                                   partition,
                                                   flags);

  testRouter(arcFlagRouter, values);
}

TEST_F(RobustArcFlagPreprocessorTest, testSelectedValuesUnidirectional)
{
  ValueVector values = {25, 15};

  RobustArcFlagPreprocessor preprocessor(graph,
                                         costs,
                                         deviations,
                                         partition,
                                         values);

  Bidirected<SimpleArcFlags> flags(graph, partition);

  preprocessor.computeFlags(flags, false);

  Dijkstra dijkstra(graph);

  SimpleThetaRouter simpleRouter(graph, costs, deviations, deviationSize);

  for(const Vertex& source : sources)
  {
    const Region& sourceRegion = partition.getRegion(source);

    for(const Vertex& target : targets)
    {
      const Region& targetRegion = partition.getRegion(target);

      for(const num& value : values)
      {
        ReducedCosts reducedCosts(costs, deviations, value);

        auto incomingResult = dijkstra.shortestPath(source,
                                                    target,
                                                    reducedCosts,
                                                    flags.get(Direction::INCOMING).getFilter(targetRegion),
                                                    inf);

        auto outgoingResult = dijkstra.shortestPath(source,
                                                    target,
                                                    reducedCosts,
                                                    flags.get(Direction::OUTGOING).getFilter(sourceRegion),
                                                    inf);

        auto simpleResult = simpleRouter.shortestPath(source,
                                                      target,
                                                      value);

        ASSERT_EQ(incomingResult.cost, simpleResult.cost);
        ASSERT_EQ(outgoingResult.cost, simpleResult.cost);
      }
    }
  }
}

TEST_F(RobustArcFlagPreprocessorTest, testImplication)
{
  ValueVector values = {25, 15};

  Bidirected<SimpleArcFlags> firstFlags(graph, partition);
  Bidirected<SimpleArcFlags> combinedFlags(graph, partition);

  {
    RobustArcFlagPreprocessor preprocessor(graph,
                                           costs,
                                           deviations,
                                           partition,
                                           {*values.begin()});

    preprocessor.computeFlags(firstFlags, false);
  }

  {
    RobustArcFlagPreprocessor preprocessor(graph,
                                           costs,
                                           deviations,
                                           partition,
                                           values);

    preprocessor.computeFlags(combinedFlags, false);
  }

  for(const Region& region : partition.getRegions())
  {
    for(const Edge& edge : graph.getEdges())
    {
      for(const num& value : values)
      {
        if(firstFlags.get(Direction::INCOMING).filter(edge, region, value))
        {
          ASSERT_TRUE(combinedFlags.get(Direction::INCOMING).filter(edge, region, value));
        }
      }
    }
  }
}

TEST_F(RobustArcFlagPreprocessorTest, testTwoValues)
{
  ValueVector values = {25, 15};

  Bidirected<SimpleArcFlags> firstFlags(graph, partition);
  Bidirected<SimpleArcFlags> combinedFlags(graph, partition);

  {
    RobustArcFlagPreprocessor preprocessor(graph,
                                           costs,
                                           deviations,
                                           partition,
                                           {*values.begin()});

    preprocessor.computeFlags(firstFlags, false);
  }

  {
    RobustArcFlagPreprocessor preprocessor(graph,
                                           costs,
                                           deviations,
                                           partition,
                                           values);

    preprocessor.computeFlags(combinedFlags, false);
  }

  num value = *(values.begin());

  ArcFlagThetaRouter<SimpleArcFlags> arcFlagRouter(graph,
                                                   costs,
                                                   deviations,
                                                   partition,
                                                   combinedFlags);

  SimpleThetaRouter simpleRouter(graph,
                                 costs,
                                 deviations,
                                 deviationSize);

  for(const Vertex& source : sources)
  {
    const Region& sourceRegion = partition.getRegion(source);

    for(const Vertex& target : targets)
    {
      const Region& targetRegion = partition.getRegion(source);

      auto arcFlagResult = arcFlagRouter.shortestPath(source, target, value);

      auto simpleResult = simpleRouter.shortestPath(source, target, value);

      if(arcFlagResult.cost != simpleResult.cost)
      {
        auto forwardFilter = combinedFlags.get(Direction::INCOMING).getThetaFilter(targetRegion, value);
        auto backwardFilter = combinedFlags.get(Direction::OUTGOING).getThetaFilter(targetRegion, value);

        Log(info) << "Testing satisfied criteria";
        bool satisfied = simpleResult.path.satisfies(forwardFilter, backwardFilter);

        if(satisfied)
        {
          Log(info) << "Path satisfies criteria";
        }
        else
        {
          Log(info) << "Path does not satisfy criteria";
        }

        ASSERT_TRUE(satisfied);
      }

      ASSERT_EQ(arcFlagResult.cost, simpleResult.cost);
    }
  }
}

TEST_F(RobustArcFlagPreprocessorTest, testUnion)
{
  ValueVector values = {25, 15};

  Bidirected<SimpleArcFlags> firstFlags(graph, partition);
  Bidirected<SimpleArcFlags> secondFlags(graph, partition);
  Bidirected<SimpleArcFlags> combinedFlags(graph, partition);

  {
    RobustArcFlagPreprocessor preprocessor(graph,
                                           costs,
                                           deviations,
                                           partition,
                                           {*values.begin()});

    preprocessor.computeFlags(firstFlags, false);
  }

  {
    RobustArcFlagPreprocessor preprocessor(graph,
                                           costs,
                                           deviations,
                                           partition,
                                           {*values.rbegin()});

    preprocessor.computeFlags(secondFlags, false);
  }

  {
    RobustArcFlagPreprocessor preprocessor(graph,
                                           costs,
                                           deviations,
                                           partition,
                                           values);

    preprocessor.computeFlags(combinedFlags, false);
  }

  for(const Region& region : partition.getRegions())
  {
    for(const Edge& edge : graph.getEdges())
    {
      for(const num& value : values)
      {
        if(firstFlags.get(Direction::INCOMING).filter(edge, region, value) or
           secondFlags.get(Direction::INCOMING).filter(edge, region, value))
        {
          ASSERT_TRUE(combinedFlags.get(Direction::INCOMING).filter(edge, region, value));
        }

        if(firstFlags.get(Direction::OUTGOING).filter(edge, region, value) or
           secondFlags.get(Direction::OUTGOING).filter(edge, region, value))
        {
          ASSERT_TRUE(combinedFlags.get(Direction::OUTGOING).filter(edge, region, value));
        }
      }
    }
  }
}

TEST_F(RobustArcFlagPreprocessorTest, testSelectedValuesBidirectional)
{
  ValueVector values = {25, 15};

  RobustArcFlagPreprocessor preprocessor(graph,
                                         costs,
                                         deviations,
                                         partition,
                                         values);

  Bidirected<SimpleArcFlags> flags(graph, partition);

  preprocessor.computeFlags(flags, false);

  BidirectionalRouter bidirectionalRouter(graph);

  SimpleThetaRouter simpleRouter(graph, costs, deviations, deviationSize);

  for(const Vertex& source : sources)
  {
    const Region& sourceRegion = partition.getRegion(source);

    for(const Vertex& target : targets)
    {
      const Region& targetRegion = partition.getRegion(target);

      for(const num& value : values)
      {
        ReducedCosts reducedCosts(costs, deviations, value);

        auto bidirectionalResult =
          bidirectionalRouter.shortestPath(source,
                                           target,
                                           reducedCosts,
                                           flags.get(Direction::INCOMING).getFilter(targetRegion),
                                           flags.get(Direction::OUTGOING).getFilter(sourceRegion),
                                           inf);

        auto simpleResult = simpleRouter.shortestPath(source,
                                                      target,
                                                      value);

        ASSERT_EQ(bidirectionalResult.cost, simpleResult.cost);
      }
    }
  }
}
