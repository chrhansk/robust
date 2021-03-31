#include "value_preprocessor_test.hh"

#include "robust/values/refining_value_preprocessor.hh"

ADD_VALUE_PREPROCESSOR_TEST(refining_value_preprocessor_test,
                            RefiningValuePreprocessor(graph, costs, deviations, deviationSize, partition))

TEST_F(ValuePreprocessorTest, test_refining_value_preprocessor)
{
  RefiningValuePreprocessor preprocessor(graph,
                                         costs,
                                         deviations,
                                         deviationSize,
                                         partition);

  RegionMap<ValueSet> values = preprocessor.requiredValues(sourceRegion);

  for(const Region& targetRegion : partition.getRegions())
  {
    if(sourceRegion == targetRegion)
    {
      continue;
    }

    auto valueSet = values(targetRegion);

    ValueVector requiredValues(valueSet.begin(), valueSet.end());

    testRouter(sourceRegion, targetRegion, requiredValues);
  }
}

