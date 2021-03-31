#include "abstract_robust_contraction_preprocessor.hh"

#include "router/bidirectional_router.hh"

const num AbstractRobustContractionPreprocessor::INVALID = (num) -1;

void AbstractRobustContractionPreprocessor::removeOverflow(
  const Graph& overlayGraph,
  EdgeMap<ContractionRange>& contractionRanges,
  const Edge& edge) const
{
  ContractionRange& range = contractionRanges(edge);

  num lowerCost = ReducedContractionCosts(contractionRanges,
                                          range.getMinimum())(edge);
  num upperCost = ReducedContractionCosts(contractionRanges,
                                          range.getMaximum())(edge);

  ValueVector& currentValues = range.getValues();
  ValueVector nextValues;

  for(const num value : currentValues)
  {
    if(value >= range.getMaximum())
    {
      ++range.getSlope();
      range.getCost() += value;
    }
    else
    {
      nextValues.push_back(value);
    }
  }

  currentValues = nextValues;

  assert(lowerCost == ReducedContractionCosts(contractionRanges,
                                              range.getMinimum())(edge));

  assert(upperCost == ReducedContractionCosts(contractionRanges,
                                              range.getMaximum())(edge));
}


void AbstractRobustContractionPreprocessor::removeOverflows(
  const Graph& overlayGraph,
  EdgeMap<ContractionRange>& contractionRanges) const
{
  for(const Edge& edge : overlayGraph.getEdges())
  {
    removeOverflow(overlayGraph, contractionRanges, edge);
  }
}

void AbstractRobustContractionPreprocessor::tightenEdges(
  const Graph& graph,
  EdgeMap<ContractionRange>& contractionRanges) const
{
  BidirectionalRouter router(graph);

  for(const Edge& edge : graph.getEdges())
  {
    ContractionRange& contractionRange = contractionRanges(edge);
    ReverseValueIterator it;
    Path lastPath;

    for(it = values.rbegin(); it != values.rend(); ++it)
    {
      num value = *it;
      ReducedContractionCosts reducedCosts(contractionRanges, value);

      EdgeValueFiter filter(deviations(edge),
                            value,
                            deviations,
                            contractionRanges);

      const num upperBound = reducedCosts(edge);

      if(lastPath)
      {
        if(lastPath.satisfies(filter) and
           lastPath.cost(reducedCosts) <= upperBound)
        {
          continue;
        }
      }

      auto result = router.shortestPath<EdgeValueFiter,
                                        EdgeValueFiter,
                                        true>(edge.getSource(),
                                              edge.getTarget(),
                                              reducedCosts,
                                              filter,
                                              filter,
                                              upperBound);

      if(result.found)
      {
        lastPath = result.path;
      }
      else
      {
        if(it != values.rbegin())
        {
          ValueIterator forward(it.base());
          contractionRange.setEnd(forward);
        }
        break;
      }
    }

    if(it == values.rend())
    {
      contractionRange.setBegin(values.end());
      contractionRange.setEnd(values.end());
    }
  }
}

void AbstractRobustContractionPreprocessor::contractVertex(
  const Vertex& vertex,
  Graph& overlayGraph,
  EdgeMap<EdgePair>& originalEdges,
  EdgeMap<ContractionRange>& contractionRanges,
  const VertexFunc<bool>& contracted,
  const std::vector<RobustContractionPair>& pairs) const
{
  for(const RobustContractionPair& pair : pairs)
  {
    const Vertex& source = pair.getSource();
    const Vertex& target = pair.getTarget();

    assert(!contracted(source));
    assert(!contracted(target));

    const Path& path = pair.getDefaultPath();
    const Edge& incoming = *(path.getEdges().begin());
    const Edge& outgoing = *(path.getEdges().rbegin());

    assert(path.getEdges().size() == 2);
    assert(path.contains(vertex));

    Edge edge = overlayGraph.addEdge(source, target);

    num cost = path.cost(OverlayCosts(contractionRanges));

    originalEdges.extend(edge,
                         EdgePair(*(path.getEdges().begin()),
                                  *(path.getEdges().rbegin())));

    contractionRanges.extend(edge,
                             ContractionRange(pair.getBegin(),
                                              pair.getEnd(),
                                              cost));

    ContractionRange& range = contractionRanges(edge);

    assert(range.getValues().empty());

    num upperBound = *(pair.getBegin());

    ReverseValueIterator rbegin(pair.getEnd());

    num lowerBound = *rbegin;

    assert(lowerBound <= upperBound);

    const ValueVector &incomingValues =
      contractionRanges(incoming).getValues();

    const ValueVector &outgoingValues =
      contractionRanges(outgoing).getValues();

    ValueVector currentValues;

    std::merge(incomingValues.begin(),
               incomingValues.end(),
               outgoingValues.begin(),
               outgoingValues.end(),
               std::back_inserter(currentValues),
               std::greater<num>());

    for(const num& value : currentValues)
    {
      if(value <= lowerBound)
      {
        break;
      }

      range.getValues().push_back(value);
    }
  }
}
