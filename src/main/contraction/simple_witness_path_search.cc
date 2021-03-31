#include "simple_witness_path_search.hh"

#include "router/bidirectional_router.hh"

std::vector<ContractionPair> SimpleWitnessPathSearch::findPairs(Vertex vertex) const
{
  std::vector<ContractionPair> pairs;

  for(const Edge& incoming : graph.getIncoming(vertex))
  {
    const Vertex& source = incoming.getSource();

    if(contracted(source) or
       source == vertex)
    {
      continue;
    }

    for(const Edge& outgoing : graph.getOutgoing(vertex))
    {
      const Vertex& target = outgoing.getTarget();

      if(contracted(target) or
         vertex == source or
         vertex == target)
      {
        continue;
      }

      const num upperBound = costs(incoming) + costs(outgoing);

      BidirectionalRouter router(graph);
      SearchPredicate predicate(contracted, vertex);

      auto result = router.shortestPath<SearchPredicate,
                                        SearchPredicate,
                                        true>(source,
                                              target,
                                              costs,
                                              predicate,
                                              predicate,
                                              upperBound);

      if(result.found)
      {
        const Path& path = result.path;

        assert(path.satisfies(predicate));
        assert(path.cost(costs) <= upperBound);
        assert(path.connects(source, target));
      }
      else
      {
        pairs.push_back(ContractionPair(source,
                                        target,
                                        Path({incoming, outgoing})));
      }
    }
  }

  assert(pairs.size() <=
         graph.getIncoming(vertex).size() *
         graph.getOutgoing(vertex).size());

  return pairs;
}
