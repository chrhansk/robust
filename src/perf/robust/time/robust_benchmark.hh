#ifndef ROBUST_BENCHMARK_HH
#define ROBUST_BENCHMARK_HH

#include "robust/robust_router.hh"
#include "robust/simple_robust_router.hh"

#include "sample_benchmark.hh"
#include "benchmark_config.hh"

class RobustBenchmark : public SampleBenchmark
{
private:
  RobustRouter& robustRouter;
public:
  RobustBenchmark(const SampleCollector& sampleCollector,
                  RobustRouter& robustRouter,
                  int minIterations,
                  double minSeconds)
    : SampleBenchmark(sampleCollector, minIterations, minSeconds),
      robustRouter(robustRouter)
  {}

  void execute(const VertexPair& sample) override
  {
    robustRouter.shortestPath(sample.source, sample.target);
  }
};

#define ROBUST_BENCHMARK(ROUTER)                                        \
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
    RobustBenchmark benchmark(sampleCollector,                          \
                              router,                                   \
                              config.getSettings().minIterations,       \
                              config.getSettings().minSeconds);         \
                                                                        \
    benchmark.executeAll();                                             \
                                                                        \
    benchmark.print(std::cout, #ROUTER);                                \
  }                                                                     \

#define THETA_BENCHMARK(ROUTER)                                         \
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
    ROUTER thetaRouter(fixture.graph,                                   \
                       fixture.costs,                                   \
                       fixture.deviations,                              \
                       config.getSettings().deviationSize);             \
                                                                        \
    SimpleRobustRouter router(fixture.graph,                            \
                              fixture.costs,                            \
                              fixture.deviations,                       \
                              config.getSettings().deviationSize,       \
                              thetaRouter);                             \
                                                                        \
    SampleCollector sampleCollector(fixture.graph,                      \
                                    fixture.costs,                      \
                                    config.getSettings().sampleSize,    \
                                    config.getSettings().numBuckets);   \
                                                                        \
    RobustBenchmark benchmark(sampleCollector,                          \
                              router,                                   \
                              config.getSettings().minIterations,       \
                              config.getSettings().minSeconds);         \
                                                                        \
    benchmark.executeAll();                                             \
                                                                        \
    benchmark.print(std::cout, #ROUTER);                                \
  }                                                                     \

#define COMBINED_BENCHMARK(ROBUST, THETA, NAME)                         \
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
    THETA thetaRouter(fixture.graph,                                    \
                       fixture.costs,                                   \
                       fixture.deviations,                              \
                       config.getSettings().deviationSize);             \
                                                                        \
    ROBUST router(fixture.graph,                                        \
                  fixture.costs,                                        \
                  fixture.deviations,                                   \
                  config.getSettings().deviationSize,                   \
                  thetaRouter);                                         \
                                                                        \
    SampleCollector sampleCollector(fixture.graph,                      \
                                    fixture.costs,                      \
                                    config.getSettings().sampleSize,    \
                                    config.getSettings().numBuckets);   \
                                                                        \
    RobustBenchmark benchmark(sampleCollector,                          \
                              router,                                   \
                              config.getSettings().minIterations,       \
                              config.getSettings().minSeconds);         \
                                                                        \
    benchmark.executeAll();                                             \
                                                                        \
    benchmark.print(std::cout, NAME);                                   \
  }                                                                     \

#endif /* ROBUST_BENCHMARK_HH */
