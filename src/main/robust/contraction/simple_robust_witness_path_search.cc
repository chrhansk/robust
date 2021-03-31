#include "simple_robust_witness_path_search.hh"

#include <cassert>

#include "router/bidirectional_router.hh"

#include "robust_search_predicate.hh"

template <class OutIt>
void SimpleRobustWitnessPathSearch::findPair(const Vertex vertex,
                                             const Edge& incoming,
                                             const Edge& outgoing,
                                             OutIt outIt) const
{
  const Vertex source = incoming.getSource();
  const Vertex target = outgoing.getTarget();

  assert(!contracted(vertex));
  assert(!contracted(incoming.getSource()));
  assert(!contracted(outgoing.getTarget()));

  ValueIterator it;

  const ContractionRange& incomingRange =
    contractionRanges(incoming);
  const ContractionRange& outgoingRange =
    contractionRanges(outgoing);

  // we only consider the intersection of
  // the respective ranges
  ValueIterator begin = std::max(incomingRange.getBegin(),
                                 outgoingRange.getBegin());

  const ValueIterator end = std::min(incomingRange.getEnd(),
                                     outgoingRange.getEnd());

  // ranges dont intersect
  if(begin >= end)
  {
    return;
  }

  // there should be at least one value in the range
  assert(begin < end);

  const Path defaultPath{incoming, outgoing};
  Path lastPath;

  for(it = begin; it != end; ++it)
  {
    const num value = *it;
    ReducedContractionCosts reducedCosts(contractionRanges, value);

    const num upperBound = defaultPath.cost(reducedCosts);

    BidirectionalRouter router(graph);
    RobustSearchPredicate predicate(contracted, contractionRanges, vertex, value);

    if(lastPath)
    {
      assert(lastPath.connects(source, target));

      if(lastPath.satisfies(predicate) and
         lastPath.cost(reducedCosts) <= upperBound)
      {
        continue;
      }
    }

    auto result = router.shortestPath<RobustSearchPredicate,
                                      RobustSearchPredicate,
                                      true>(source,
                                            target,
                                            reducedCosts,
                                            predicate,
                                            predicate,
                                            upperBound);

    if(result.found)
    {
      assert(result.path.connects(source, target));
      assert(result.path.cost(reducedCosts) <= upperBound);
      assert(result.path.satisfies(predicate));
    }
    else
    {
      break;
    }

    lastPath = result.path;
  }

  // witness paths were found for *all* values
  if(it == end)
  {
    return;
  }

  begin = it;
  assert(begin < end);

  ReverseValueIterator rbegin(end), rend(++it), rit;

  for(rit = rbegin; rit != rend; ++rit)
  {
    const num value = *rit;
    ReducedContractionCosts reducedCosts(contractionRanges, value);

    const num upperBound = defaultPath.cost(reducedCosts);

    BidirectionalRouter router(graph);
    RobustSearchPredicate predicate(contracted, contractionRanges, vertex, value);

    if(lastPath)
    {
      assert(lastPath.connects(source, target));

      if(lastPath.satisfies(predicate) and
         lastPath.cost(reducedCosts) <= upperBound)
      {
        continue;
      }
    }

    auto result = router.shortestPath<RobustSearchPredicate,
                                      RobustSearchPredicate,
                                      true>(source,
                                            target,
                                            reducedCosts,
                                            predicate,
                                            predicate,
                                            upperBound);

    if(result.found)
    {
      assert(result.path.connects(source, target));
      assert(result.path.cost(reducedCosts) <= upperBound);
      assert(result.path.satisfies(predicate));
    }
    else
    {
      break;
    }

    lastPath = result.path;
  }

  assert(rit <= rend);

  ValueIterator tmp;

  if(rit < rend)
  {
    tmp = rit.base();
  }
  else
  {
    tmp = begin;
    ++tmp;
  }

  *outIt++ = RobustContractionPair(source,
                                   target,
                                   defaultPath,
                                   begin,
                                   tmp);
}

std::vector<RobustContractionPair>
SimpleRobustWitnessPathSearch::findPairs(Vertex vertex) const
{
  std::vector<RobustContractionPair> pairs;


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
         source == target or
         vertex == target)
      {
        continue;
      }

      findPair(vertex, incoming, outgoing, std::back_inserter(pairs));
    }
  }

  assert(pairs.size() <=
         graph.getIncoming(vertex).size() *
         graph.getOutgoing(vertex).size());

  return pairs;
}
