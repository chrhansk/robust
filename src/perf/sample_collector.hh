#ifndef SAMPLE_COLLECTOR_HH
#define SAMPLE_COLLECTOR_HH

#include "util.hh"

#include "graph/graph.hh"
#include "graph/edge_map.hh"

class VertexPair
{
public:
  VertexPair()
  {}
  VertexPair(Vertex source, Vertex target)
    : source(source),
      target(target)
  {}
  Vertex source, target;
};

class SampleCollector
{
private:
  std::vector<std::vector<VertexPair>> samples;
  const uint sampleSize;
  const uint numBuckets;
public:
  SampleCollector(const Graph& graph,
                  const EdgeFunc<num>& costs,
                  uint sampleSize,
                  uint numBuckets);

  uint getNumBuckets() const
  {
    return numBuckets;
  }

  uint getSampleSize() const
  {
    return sampleSize;
  }

  const std::vector<std::vector<VertexPair>>& getSamples() const
  {
    return samples;
  }

  const std::vector<VertexPair>& getSamples(uint bucket) const
  {
    return samples[bucket];
  }
};


#endif /* SAMPLE_COLLECTOR_HH */
