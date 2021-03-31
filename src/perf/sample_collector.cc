#include "sample_collector.hh"

#include "log.hh"

#include "router/dijkstra_rank.hh"

SampleCollector::SampleCollector(const Graph& graph,
                                 const EdgeFunc<num>& costs,
                                 uint sampleSize,
                                 uint numBuckets)
  : sampleSize(sampleSize),
    numBuckets(numBuckets)
{
  samples.resize(numBuckets);

  for(uint i = 0; i < numBuckets; ++i)
  {
    samples[i] = std::vector<VertexPair>();
    samples[i].reserve(sampleSize);
  }

  std::vector<Vertex> vertices = graph.getVertices().collect();
  const uint n = vertices.size();

  Log(info) << "Collecting samples of size " << sampleSize
            << " into " << numBuckets
            << " individual buckets";

  shuffle(vertices.begin(), vertices.end());

  assert(sampleSize <= n);
  assert(numBuckets <= n);

  auto it = vertices.begin();

  for(uint i = 0; i < sampleSize; ++i)
  {
    const Vertex& source = *it;

    std::vector<Vertex> nearest = nearestVertices(graph, source, costs);

    assert(nearest.size() == n);

    for(uint j = 0; j < numBuckets; ++j)
    {
      const Vertex& target = nearest.at(std::ceil(j * n / ((float)numBuckets)));
      samples[j].push_back(VertexPair(source, target));
    }

    ++it;
  }

}
