#include "robust_contraction_pair.hh"

#include "robust_contraction_preprocessor.hh"
#include "robust_search_predicate.hh"

#include "router/bidirectional_router.hh"

RobustContractionPair::RobustContractionPair(Vertex source,
                                             Vertex target,
                                             const Path& defaultPath,
                                             std::vector<num>::const_iterator begin,
                                             std::vector<num>::const_iterator end)
  : source(source),
    target(target),
    defaultPath(defaultPath),
    begin(begin),
    end(end)
{
  assert(defaultPath.getEdges().size() == 2);
  assert(begin < end);
}

bool RobustContractionPair::check(const Graph& graph,
                                  const ValueVector& values,
                                  const VertexFunc<bool>& contracted,
                                  const EdgeFunc<const ContractionRange&>& contractionRanges) const
{
  const Edge& incoming = *(defaultPath.getEdges().begin());
  const Edge& outgoing = *(defaultPath.getEdges().rbegin());

  const Vertex vertex = incoming.getTarget();

  if(getBegin() < values.begin() or getBegin() > values.end())
  {
    return false;
  }

  if(getEnd() < values.begin() or getEnd() > values.end())
  {
    return false;
  }

  ValueIterator begin, end, it;

  begin = std::max(contractionRanges(incoming).getBegin(),
                   contractionRanges(outgoing).getBegin());

  end = std::min(contractionRanges(incoming).getEnd(),
                 contractionRanges(outgoing).getEnd());

  if(begin > getBegin() or
     end < getEnd())
  {
    return false;
  }

  BidirectionalRouter router(graph);

  for(auto it = begin; it != getBegin(); ++it)
  {
    const num value = *it;
    ReducedContractionCosts costs(contractionRanges, value);
    RobustSearchPredicate predicate(contracted, contractionRanges, vertex, value);

    const num upperBound = defaultPath.cost(costs);

    BidirectionalRouter router(graph);

    auto result = router.shortestPath<RobustSearchPredicate,
                                      RobustSearchPredicate,
                                      true>(defaultPath.getSource(),
                                            defaultPath.getTarget(),
                                            costs,
                                            predicate,
                                            predicate,
                                            upperBound);

    if(!result.found)
    {
      return false;
    }

  }

  for(auto it = getEnd(); it != end; ++it)
  {
    const num value = *it;
    ReducedContractionCosts costs(contractionRanges, value);
    RobustSearchPredicate predicate(contracted, contractionRanges, vertex, value);

    const num upperBound = defaultPath.cost(costs);

    auto result = router.shortestPath<RobustSearchPredicate,
                                      RobustSearchPredicate,
                                      true>(defaultPath.getSource(),
                                            defaultPath.getTarget(),
                                            costs,
                                            predicate,
                                            predicate,
                                            upperBound);

    if(!result.found)
    {
      return false;
    }
  }

  num value = *getBegin();

  ReducedContractionCosts costs(contractionRanges, value);
  RobustSearchPredicate predicate(contracted, contractionRanges, vertex, value);

  const num upperBound = defaultPath.cost(costs);

  auto result = router.shortestPath<RobustSearchPredicate,
                                    RobustSearchPredicate,
                                    true>(defaultPath.getSource(),
                                          defaultPath.getTarget(),
                                          costs,
                                          predicate,
                                          predicate,
                                          upperBound);

  if(result.found)
  {
    return false;
  }

  const idx dist = std::distance(getBegin(), getEnd());

  if(dist > 1)
  {
    ReverseValueIterator rbegin(getEnd());
    const num value = *rbegin;

    ReducedContractionCosts costs(contractionRanges, value);
    RobustSearchPredicate predicate(contracted, contractionRanges, vertex, value);

    const num upperBound = defaultPath.cost(costs);

    auto result = router.shortestPath<RobustSearchPredicate,
                                      RobustSearchPredicate,
                                      true>(defaultPath.getSource(),
                                            defaultPath.getTarget(),
                                            costs,
                                            predicate,
                                            predicate,
                                            upperBound);

    if(result.found)
    {
      return false;
    }
  }

  return true;
}
