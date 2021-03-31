#include "sample_benchmark.hh"

#include <fstream>
#include <sstream>

#include "log.hh"
#include "util.hh"

#include "reader/graph_reader.hh"

SimpleGraphFixture::SimpleGraphFixture(const std::string& instance)
{
  std::string directory = BASE_DIRECTORY;
  std::string filename = directory + "/" + instance + ".pbf";

  Log(info) << "Opening " << filename;

  std::ifstream input(filename);

  ReadResult result = GraphReader().readGraph(input);

  graph = result.graph;
  costMap = result.costs;
  deviationMap = result.deviations;
}


GraphFixture::GraphFixture(const std::string& instance)
  : SimpleGraphFixture(instance),
    costs(costMap.getValues()),
    deviations(deviationMap.getValues())
{

}

SampleBenchmark::SampleBenchmark(const SampleCollector& sampleCollector,
                                 int minIterations,
                                 double minSeconds)
  : sampleCollector(sampleCollector),
    minIterations(minIterations),
    minSeconds(minSeconds)
{}


void SampleBenchmark::executeAll()
{
  const uint numBuckets = sampleCollector.getNumBuckets();

  results.reserve(numBuckets);

  for(uint bucket = 0; bucket < numBuckets; ++bucket)
  {
    const std::vector<VertexPair>& currentSamples = sampleCollector.getSamples(bucket);

    auto begin = currentSamples.begin();
    auto end = currentSamples.end();

    auto func = [&] (const VertexPair& sample) {
      execute(sample);
    };

    RangedBenchmark<decltype(begin), decltype(func)> benchmark(begin,
                                                               end,
                                                               minIterations,
                                                               minSeconds,
                                                               func);

    results.push_back(benchmark.execute());
  }
}

void SampleBenchmark::print(std::ostream& out,
                            const std::string& name)
{
  std::stringstream stream;

  stream << "Rank, Values" << std::endl;

  const uint numBuckets = sampleCollector.getNumBuckets();

  for(uint bucket = 0; bucket < sampleCollector.getNumBuckets(); ++bucket)
  {
    const double rank = round(100 * (bucket / ((double) numBuckets))) / 100.;

    stream << rank << ", ";

    const auto& result = results.at(bucket);

    for(idx i = 0; i < result.results.size(); ++i)
    {
      stream << result.results[i].averageSeconds();
      if(i < result.results.size() - 1)
      {
        stream << "; ";
      }
    }


    stream << std::endl;

    /*
    stream << results.at(bucket).averageSeconds << ", ";
    stream << results.at(bucket).maxSeconds - average << ", ";
    stream << average - results.at(bucket).minSeconds << std::endl;
    */
  }

  out << stream.str();
}
