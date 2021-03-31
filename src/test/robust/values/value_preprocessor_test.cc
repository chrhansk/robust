#include "value_preprocessor_test.hh"

#include "robust/theta/simple_theta_router.hh"
#include "robust/simple_robust_router.hh"
#include "robust/robust_costs.hh"

ValuePreprocessorTest::ValuePreprocessorTest()
  : partition(graph, 64),
    sourceRegion(*partition.getRegions().begin()),
    targetRegion(*partition.getRegions().rbegin())
{
  assert(sourceRegion != targetRegion);
}

void
ValuePreprocessorTest::testRouter(const ValueVector& requiredValues)
{
  testRouter(sourceRegion, targetRegion, requiredValues);
}

void
ValuePreprocessorTest::testRouter(const Region& sourceRegion,
                                  const Region& targetRegion,
                                  const ValueVector& requiredValues)
{
  SimpleThetaRouter thetaRouter(graph,
                                costs,
                                deviations,
                                deviationSize);

  testRouter(sourceRegion, targetRegion, thetaRouter, requiredValues);
}


void
ValuePreprocessorTest::testRouter(const Region& sourceRegion,
                                  const Region& targetRegion,
                                  ThetaRouter& thetaRouter,
                                  const ValueVector& requiredValues)
{
  assert(sourceRegion != targetRegion);

  SimpleRobustRouter router(graph,
                            costs,
                            deviations,
                            deviationSize,
                            thetaRouter,
                            false);

  RobustCosts robustCosts(costs, deviations, deviationSize);

  idx i = 0, j = 0;

  for(const Vertex& source : sourceRegion.getVertices())
  {
    if(++i >= 10)
    {
      break;
    }

    for(const Vertex& target : targetRegion.getVertices())
    {
      if(++j >= 10)
      {
        break;
      }

      RobustSearchResult expected = router.shortestPath(source, target);
      RobustSearchResult actual = router.shortestPath(source, target, requiredValues, inf);

      ASSERT_EQ(expected.found, actual.found);

      if(expected.found)
      {
        ASSERT_EQ(robustCosts.get(actual.path), robustCosts.get(expected.path));
      }
    }
  }
}
