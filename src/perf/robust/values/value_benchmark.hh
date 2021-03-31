#ifndef VALUE_BENCHMARK_HH
#define VALUE_BENCHMARK_HH

#include "log.hh"

#include "robust/robust_router.hh"
#include "robust/theta/simple_theta_router.hh"

#include "benchmark_config.hh"
#include "benchmark.hh"
#include "sample_collector.hh"
#include "sample_benchmark.hh"

#include <iostream>

struct ValueResult
{
  int minNumValues;
  int maxNumValues;
  double averageNumValues;
};

class ValueBenchmark
{
private:
  const SampleCollector& sampleCollector;
  RobustRouter& robustRouter;

  std::vector<ValueResult> results;

  ValueResult execute(const std::vector<VertexPair>& samples);

public:
  ValueBenchmark(const SampleCollector& sampleCollector,
                 RobustRouter& robustRouter)
    : sampleCollector(sampleCollector),
      robustRouter(robustRouter)
  {}

  void executeAll();

  void print(std::ostream& out, const std::string& name);

};

#define SIMPLE_VALUE_BENCHMARK(ROUTER)                                  \
  int main(int argc, char** argv)                                       \
  {                                                                     \
    logInit();                                                          \
                                                                        \
    std::string configName = "benchmark.json";                          \
                                                                        \
    if(argc > 1)                                                        \
    {                                                                   \
      configName = argv[1];                                             \
    }                                                                   \
                                                                        \
    BenchmarkConfig config =                                            \
      BenchmarkConfig::readConfig(configName);                          \
                                                                        \
    GraphFixture fixture(config.getInstance());                         \
                                                                        \
    ROUTER router(fixture.graph,                                        \
                  fixture.costs,                                        \
                  fixture.deviations,                                   \
                  config.getSettings().deviationSize);                  \
                                                                        \
    SampleCollector sampleCollector(fixture.graph,                      \
                                    fixture.costs,                      \
                                    config.getSettings().sampleSize,    \
                                    config.getSettings().numBuckets);   \
                                                                        \
    ValueBenchmark benchmark(sampleCollector,                           \
                             router);                                   \
                                                                        \
    benchmark.executeAll();                                             \
                                                                        \
    benchmark.print(std::cout, #ROUTER);                                \
  }                                                                     \


#define VALUE_BENCHMARK(ROUTER)                                         \
  int main(int argc, char** argv)                                       \
  {                                                                     \
    logInit();                                                          \
                                                                        \
    std::string configName = "benchmark.json";                          \
                                                                        \
    if(argc > 1)                                                        \
    {                                                                   \
      configName = argv[1];                                             \
    }                                                                   \
                                                                        \
    BenchmarkConfig config =                                            \
      BenchmarkConfig::readConfig(configName);                          \
                                                                        \
    GraphFixture fixture(config.getInstance());                         \
                                                                        \
    SimpleThetaRouter thetaRouter(fixture.graph,                        \
                                  fixture.costs,                        \
                                  fixture.deviations,                   \
                                  config.getSettings().deviationSize);  \
                                                                        \
    ROUTER robustRouter(fixture.graph,                                  \
                        fixture.costs,                                  \
                        fixture.deviations,                             \
                        config.getSettings().deviationSize,             \
                        thetaRouter);                                   \
                                                                        \
    SampleCollector sampleCollector(fixture.graph,                      \
                                    fixture.costs,                      \
                                    config.getSettings().sampleSize,    \
                                    config.getSettings().numBuckets);   \
                                                                        \
    ValueBenchmark benchmark(sampleCollector,                           \
                             robustRouter);                             \
                                                                        \
    benchmark.executeAll();                                             \
                                                                        \
    benchmark.print(std::cout, #ROUTER);                                \
  }                                                                     \

#endif /* VALUE_BENCHMARK_HH */
