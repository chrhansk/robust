#include "arcflags/arcflags.hh"

#include "robust/values/outer_value_preprocessor.hh"

#include "robust/arcflags/simple_arcflags.hh"
#include "robust/arcflags/value_arcflag_preprocessor.hh"

#include "robust/values/value_preprocessor_test.hh"

TEST_F(ValuePreprocessorTest, testValueArcFlagPreprocessor)
{
  OuterValuePreprocessor preprocessor(graph, costs, deviations, deviationSize, partition);

  const Region& sourceRegion = *(partition.getRegions().begin());

  RegionPairMap<ValueVector> requiredValues;

  for(const Region& targetRegion : partition.getRegions())
  {
    if(sourceRegion == targetRegion)
    {
      continue;
    }

    ValueSet valueSet = preprocessor.requiredValues(sourceRegion,
                                                    targetRegion);

    ValueVector values(valueSet.begin(), valueSet.end());

    requiredValues.put(sourceRegion, targetRegion, values);
  }

  SimpleArcFlags flags(graph, partition);

  ValueArcFlagPreprocessor arcFlagPreprocessor(graph, costs, deviations, partition);

  arcFlagPreprocessor.computeFlags<Direction::OUTGOING>(sourceRegion, requiredValues, flags);

  // first test whether the values alone are working
  testRouter(requiredValues(sourceRegion, targetRegion));

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

      Dijkstra dijkstra(graph);

      for(const num& value : requiredValues(sourceRegion, targetRegion))
      {
        RobustArcFlags::ThetaFilter filter = flags.getThetaFilter(sourceRegion, value);
        ReducedCosts reducedCosts(costs, deviations, deviationSize);

        auto expected = dijkstra.shortestPath<AllEdgeFilter, false>(source,
                                                                    target,
                                                                    reducedCosts,
                                                                    AllEdgeFilter(),
                                                                    inf);

        auto actual = dijkstra.shortestPath<RobustArcFlags::ThetaFilter, false>(source,
                                                                                target,
                                                                                reducedCosts,
                                                                                filter,
                                                                                inf);

        ASSERT_TRUE(expected.found);
        ASSERT_TRUE(actual.found);

        ASSERT_EQ(expected.path.cost(reducedCosts), actual.path.cost(reducedCosts));

      }
    }
  }
}
