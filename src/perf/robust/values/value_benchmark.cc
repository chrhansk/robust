#include "value_benchmark.hh"

#include <sstream>

void ValueBenchmark::executeAll()
{
  const uint numBuckets = sampleCollector.getNumBuckets();

  results.reserve(numBuckets);

  for(uint bucket = 0; bucket < numBuckets; ++bucket)
  {
    const std::vector<VertexPair>& currentSamples = sampleCollector.getSamples(bucket);

    results.push_back(execute(currentSamples));
  }
}


ValueResult ValueBenchmark::execute(const std::vector<VertexPair>& samples)
{
  int minValues = std::numeric_limits<int>::max();
  int maxValues = std::numeric_limits<int>::min();
  double averageValues = 0;

  for(const VertexPair& sample : samples)
  {
    auto result = robustRouter.shortestPath(sample.source, sample.target);

    minValues = std::min(minValues, (int) result.calls);
    maxValues = std::max(maxValues, (int) result.calls);
    averageValues += result.calls;
  }

  averageValues /= (double (samples.size()));

  return ValueResult{minValues, maxValues, averageValues};
}


void ValueBenchmark::print(std::ostream& out, const std::string& name)
{
  std::stringstream stream;
  stream << "Name";

  for(uint bucket = 0; bucket < sampleCollector.getNumBuckets(); ++bucket)
  {
    stream << ", ";
    stream << bucket;
  }

  out << stream.str() << "\n";

  stream.str("");

  stream << name << "Min";

  for(uint bucket = 0; bucket < sampleCollector.getNumBuckets(); ++bucket)
  {
    stream << ", ";
    stream << results.at(bucket).minNumValues;
  }

  out << stream.str() << "\n";

  stream.str("");

  stream << name << "Max";

  for(uint bucket = 0; bucket < sampleCollector.getNumBuckets(); ++bucket)
  {
    stream << ", ";
    stream << results.at(bucket).maxNumValues;
  }

  out << stream.str() << "\n";

  stream.str("");

  stream << name << "Avg";

  for(uint bucket = 0; bucket < sampleCollector.getNumBuckets(); ++bucket)
  {
    stream << ", ";
    stream << results.at(bucket).averageNumValues;
  }

  out << stream.str() << "\n";
}
