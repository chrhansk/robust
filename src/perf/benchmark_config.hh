#ifndef BENCHMARK_CONFIG_HH
#define BENCHMARK_CONFIG_HH

#include <string>

struct BenchmarkSettings
{
  int minIterations;
  double minSeconds;

  int numBuckets;
  int sampleSize;

  int deviationSize;
};

class BenchmarkConfig
{
private:
  std::string instance;
  BenchmarkSettings settings;

public:
  static BenchmarkConfig readConfig(const std::string& filename);

  const BenchmarkSettings& getSettings() const
  {
    return settings;
  }

  const std::string& getInstance() const
  {
    return instance;
  }
};




#endif /* BENCHMARK_CONFIG_HH */
