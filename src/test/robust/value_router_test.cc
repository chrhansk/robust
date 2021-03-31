#include "robust_router_test.hh"

#include <fstream>

#include "reader/required_values_reader.hh"

#include "robust/theta/simple_theta_router.hh"
#include "robust/simple_robust_router.hh"
#include "robust/values/robust_value_router.hh"

class ValueRouterTest : public RobustRouterTest
{
protected:
  std::unique_ptr<RegionPairMap<ValueVector>> requiredValues;
  std::unique_ptr<Partition> partition;
  idx deviationSize;
public:
  ValueRouterTest();
};

ValueRouterTest::ValueRouterTest()
  : RobustRouterTest()
{
  std::string directory = BASE_DIRECTORY;

  std::ifstream valuesInput(directory + "/" + INSTANCE + ".values.pbf");

  RequiredValuesReadResult valuesResult = RequiredValuesReader().readRequiredValues(graph,
                                                                                    valuesInput);

  partition = std::move(valuesResult.partition);

  requiredValues = std::move(valuesResult.requiredValues);

  costMap = valuesResult.costs;
  deviationMap = valuesResult.deviations;
  costs = costMap.getValues();
  deviations = deviationMap.getValues();
  deviationSize = valuesResult.deviationSize;
}

TEST_F(ValueRouterTest, testSimple)
{
  SimpleThetaRouter thetaRouter(graph, costs, deviations);

  SimpleRobustRouter router(graph,
                            costs,
                            deviations,
                            deviationSize,
                            thetaRouter,
                            true);

  RobustValueRouter valueRouter(costs,
                                deviations,
                                deviationSize,
                                router,
                                *partition,
                                *requiredValues);

  testRobustRouter(valueRouter);
}

