#ifndef SAMPLE_BENCHMARK_HH
#define SAMPLE_BENCHMARK_HH

#include <iostream>

#include "util.hh"

#include "graph/graph.hh"
#include "graph/edge_map.hh"

#include "sample_collector.hh"
#include "benchmark.hh"

class SimpleGraphFixture
{
public:
  SimpleGraphFixture(const std::string& instance);

  Graph graph;
  EdgeMap<num> costMap;
  EdgeMap<num> deviationMap;
};

class GraphFixture : public SimpleGraphFixture
{
public:
  GraphFixture(const std::string& instance);

  EdgeValueMap<num> costs;
  EdgeValueMap<num> deviations;
};

class SampleBenchmark
{
protected:
  const SampleCollector& sampleCollector;

  std::vector<RangedResult> results;

  int minIterations;
  double minSeconds;

public:
  SampleBenchmark(const SampleCollector& sampleCollector,
                  int minIterations,
                  double minSeconds);

  virtual void execute(const VertexPair& sample) = 0;

  void executeAll();

  void print(std::ostream& out,
             const std::string& name);

};


#endif /* SAMPLE_BENCHMARK_HH */
