#include "hop_restricted_witness_path_search.hh"

#include "router/label_heap.hh"

class HopRestrictedLabel : public SimpleLabel
{
private:
  idx hops;

public:
  HopRestrictedLabel(Vertex vertex, num cost, idx hops)
    : SimpleLabel(vertex, cost),
      hops(hops) {}
  HopRestrictedLabel() {}

  idx getHops() const
  {
    return hops;
  }
};

HopRestrictedWitnessPathSearch::HopRestrictedWitnessPathSearch(
  const Graph& graph,
  const EdgeFunc<num>& costs,
  const VertexFunc<bool>& contracted,
  idx hopLimit)
  : WitnessPathSearch(graph, costs, contracted),
    hopLimit(hopLimit)
{
  assert(hopLimit > 0);
}

std::vector<ContractionPair> HopRestrictedWitnessPathSearch::findPairs(
  Vertex vertex) const
{
  const std::vector<Edge>& incoming = graph.getIncoming(vertex);
  const std::vector<Edge>& outgoing = graph.getOutgoing(vertex);

  std::vector<Edge> actualIncoming, actualOutgoing;

  std::copy_if(incoming.begin(),
               incoming.end(),
               std::back_inserter(actualIncoming),
               [this, &vertex](const Edge& edge) -> bool
               {
                 const Vertex& source = edge.getSource();
                 return !(contracted(source) or
                          source == vertex);
               });

  std::copy_if(outgoing.begin(),
               outgoing.end(),
               std::back_inserter(actualOutgoing),
               [this, &vertex](const Edge& edge) -> bool
               {
                 const Vertex& target = edge.getTarget();
                 return !(contracted(target) or
                          target == vertex);
               });

  if(actualIncoming.empty() or actualOutgoing.empty())
  {
    return {};
  }

  std::vector<ContractionPair> pairs;

  if(actualIncoming.size() < actualOutgoing.size())
  {
    for(const Edge& incomingEdge : actualIncoming)
    {
      assert(incomingEdge.getTarget() == vertex);
      findPairs<Direction::OUTGOING>(vertex,
                                     incomingEdge,
                                     actualOutgoing,
                                     std::back_inserter(pairs));
    }
  }
  else
  {
    for(const Edge& outgoingEdge : actualOutgoing)
    {
      assert(outgoingEdge.getSource() == vertex);
      findPairs<Direction::INCOMING>(vertex,
                                     outgoingEdge,
                                     actualIncoming,
                                     std::back_inserter(pairs));
    }
  }

  return pairs;
}

template<Direction direction, class OutIt>
void HopRestrictedWitnessPathSearch::findPairs(const Vertex vertex,
                                               const Edge edge,
                                               const std::vector<Edge>& edges,
                                               OutIt it) const
{
  assert(edge.getEndpoint(direction) == vertex);

  LabelHeap<HopRestrictedLabel> labelHeap(graph);
  SearchPredicate predicate(contracted, vertex);

  Vertex source = edge.getEndpoint(opposite(direction));

  labelHeap.update(HopRestrictedLabel(source, 0, 0));

  for(const Edge& other : edges)
  {
    Path defaultPath;

    if(direction == Direction::OUTGOING)
    {
      defaultPath = Path{edge, other};
    }
    else
    {
      defaultPath = Path{other, edge};
    }

    assert(defaultPath.connects(source,
                                other.getEndpoint(direction),
                                direction));

    num upperBound = defaultPath.cost(costs);

    const Vertex target = defaultPath.getEndpoint(direction);

    bool found = false;

    while(!labelHeap.isEmpty())
    {
      const HopRestrictedLabel& targetLabel = labelHeap.getLabel(target);

      if(targetLabel.getState() != State::UNKNOWN)
      {
        if(targetLabel.getCost() <= upperBound)
        {
          found = true;
          break;
        }
      }

      if(labelHeap.peek().getCost() > upperBound or
         labelHeap.peek().getHops() == hopLimit)
      {
        *it++ = ContractionPair(defaultPath.getSource(),
                                defaultPath.getTarget(),
                                defaultPath);

        found = true;
        break;
      }

      const HopRestrictedLabel& currentLabel = labelHeap.extractMin();
      Vertex currentVertex = currentLabel.getVertex();

      for(const Edge& outEdge : graph.getEdges(currentVertex, direction))
      {
        if(!predicate(outEdge))
        {
          continue;
        }

        Vertex nextVertex = outEdge.getEndpoint(direction);
        num nextCost = currentLabel.getCost() + costs(outEdge);

        labelHeap.update(HopRestrictedLabel(nextVertex,
                                            nextCost,
                                            currentLabel.getHops() + 1));
      }
    }

    if(!found)
    {
      const SimpleLabel& targetLabel = labelHeap.getLabel(target);

      if(targetLabel.getState() == State::UNKNOWN or
         targetLabel.getCost() > upperBound)
      {
        *it++ = ContractionPair(defaultPath.getSource(),
                                defaultPath.getTarget(),
                                defaultPath);
      }
    }
  }
}
