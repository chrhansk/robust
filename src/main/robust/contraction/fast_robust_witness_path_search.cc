#include "fast_robust_witness_path_search.hh"

#include "router/label.hh"
#include "router/label_heap.hh"

#include "robust_search_predicate.hh"

class SearchData
{
private:
  Path defaultPath, lastPath;
  ValueIterator begin, end;
  ValueIterator failed;
public:
  SearchData(Path defaultPath,
             ValueIterator begin,
             ValueIterator end)
    : defaultPath(defaultPath),
      begin(begin),
      end(end)
  {}

  const ValueIterator& getBegin() const
  {
    return begin;
  }

  const ValueIterator& getEnd() const
  {
    return end;
  }

  const Path& getDefaultPath() const
  {
    return defaultPath;
  }

  Path& getLastPath()
  {
    return lastPath;
  }

  ValueIterator& failedIterator()
  {
    return failed;
  }

};

template <Direction direction, class Predicate>
Path searchPath(const Graph& graph,
                Vertex source,
                Vertex target,
                num upperBound,
                const Predicate& predicate,
                const EdgeFunc<num>& costs,
                LabelHeap<Label>& labelHeap)
{
  while(true)
  {
    const Label& targetLabel = labelHeap.getLabel(target);

    if(targetLabel.getState() != State::UNKNOWN)
    {
      if(targetLabel.getCost() <= upperBound)
      {
        Path path;
        Label current = targetLabel;

        while(current.getVertex() != source)
        {
          Edge edge = current.getEdge();
          path.add(edge, opposite(direction));
          current = labelHeap.getLabel(edge.getEndpoint(opposite(direction)));
        }

        return path;
      }
    }

    if(labelHeap.isEmpty() or
       labelHeap.peek().getCost() > upperBound)
    {
      break;
    }

    const Label& currentLabel = labelHeap.extractMin();
    Vertex currentVertex = currentLabel.getVertex();

    for(const Edge& outEdge : graph.getEdges(currentVertex,
                                             direction))
    {
      if(!predicate(outEdge))
      {
        continue;
      }

      Vertex nextVertex = outEdge.getEndpoint(direction);
      num nextCost = currentLabel.getCost() + costs(outEdge);

      labelHeap.update(Label(nextVertex, outEdge, nextCost));
    }
  }

  return Path();
}

std::vector<RobustContractionPair>
FastRobustWitnessPathSearch::findPairs(Vertex vertex) const
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

  std::vector<RobustContractionPair> pairs;

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


template <Direction direction, class OutIt>
void FastRobustWitnessPathSearch::findPairs(const Vertex vertex,
                                            const Edge edge,
                                            const std::vector<Edge>& edges,
                                            OutIt outIt) const
{
  ValueIterator begin = values.end(),
    end = values.begin();

  const Vertex source = edge.getEndpoint(opposite(direction));

  std::vector<SearchData> searchData, nextData, failedData;

  for(const Edge& other : edges)
  {
    const ContractionRange& otherRange = contractionRanges(other);
    begin = std::min(begin, otherRange.getBegin());
    end = std::max(end, otherRange.getEnd());

    Path defaultPath;
    Vertex target = other.getEndpoint(direction);

    if(source == target)
    {
      continue;
    }

    if(direction == Direction::OUTGOING)
    {
      defaultPath = Path{edge, other};
    }
    else
    {
      defaultPath = Path{other, edge};
    }

    assert(defaultPath);
    assert(defaultPath.connects(source,
                                target,
                                direction));

    searchData.push_back(SearchData(defaultPath,
                                    otherRange.getBegin(),
                                    otherRange.getEnd()));
  }

  const ContractionRange& edgeRange = contractionRanges(edge);

  begin = std::max(begin, edgeRange.getBegin());
  end = std::min(end, edgeRange.getEnd());

  if(begin >= end)
  {
    return;
  }

  assert(begin < end);

  ValueIterator it;

  // We first search upwards through the values...
  for(it = begin; it != end; ++it)
  {
    num value = *it;
    RobustSearchPredicate predicate(contracted, contractionRanges, vertex, value);
    ReducedContractionCosts costs(contractionRanges, value);

    LabelHeap<Label> labelHeap(graph);
    labelHeap.update(Label(source, Edge(), 0));

    for(SearchData& currentData : searchData)
    {
      if(it < currentData.getBegin())
      {
        nextData.push_back(currentData);
        continue;
      }
      else if(it >= currentData.getEnd())
      {
        // disregard completely, witness path partition found...
        continue;
      }

      const Path& defaultPath = currentData.getDefaultPath();
      const num upperBound = defaultPath.cost(costs);
      Path& lastPath = currentData.getLastPath();

      if(lastPath)
      {
        if(lastPath.satisfies(predicate) and
           lastPath.cost(costs) <= upperBound)
        {
          nextData.push_back(currentData);
          continue;
        }
      }

      const Vertex target = defaultPath.getEndpoint(direction);

      assert(target != source);

      Path path = searchPath<direction, RobustSearchPredicate>(graph,
                                                               source,
                                                               target,
                                                               upperBound,
                                                               predicate,
                                                               costs,
                                                               labelHeap);

      if(path)
      {
        lastPath = path;
        nextData.push_back(currentData);
      }
      else
      {
        currentData.failedIterator() = it;
        failedData.push_back(currentData);
      }
    }

    searchData.clear();
    std::swap(searchData, nextData);
  }

  if(failedData.empty())
  {
    return;
  }

  begin = values.end();

  for(SearchData& currentData : failedData)
  {
    begin = std::min(begin, currentData.failedIterator());
  }

  ReverseValueIterator rbegin(end), rend(begin), rit;

  // now we search backwards...
  for(rit = rbegin; rit != rend; ++rit)
  {
    num value = *rit;
    RobustSearchPredicate predicate(contracted, contractionRanges, vertex, value);
    ReducedContractionCosts costs(contractionRanges, value);

    LabelHeap<Label> labelHeap(graph);
    labelHeap.update(Label(source, Edge(), 0));

    for(SearchData& currentData : failedData)
    {
      if(rit < ReverseValueIterator(currentData.getEnd()))
      {
        nextData.push_back(currentData);
        continue;
      }

      const Path& defaultPath = currentData.getDefaultPath();
      num upperBound = defaultPath.cost(costs);

      Path& lastPath = currentData.getLastPath();

      if(lastPath)
      {
        if(lastPath.satisfies(predicate) and
           lastPath.cost(costs) <= upperBound)
        {
          nextData.push_back(currentData);
          continue;
        }
      }

      const Vertex target = defaultPath.getEndpoint(direction);

      Path path = searchPath<direction, RobustSearchPredicate>(graph,
                                                               source,
                                                               target,
                                                               upperBound,
                                                               predicate,
                                                               costs,
                                                               labelHeap);

      if(path)
      {
        lastPath = path;
        nextData.push_back(currentData);
      }
      else
      {
        *outIt++ = RobustContractionPair(defaultPath.getSource(),
                                         defaultPath.getTarget(),
                                         defaultPath,
                                         currentData.failedIterator(),
                                         ValueIterator(rit.base()));
      }
    }

    failedData.clear();
    std::swap(failedData, nextData);
  }

  for(SearchData& currentData : failedData)
  {
    const Path& defaultPath = currentData.getDefaultPath();

    *outIt++ = RobustContractionPair(defaultPath.getSource(),
                                     defaultPath.getTarget(),
                                     defaultPath,
                                     currentData.failedIterator(),
                                     ValueIterator(rit.base()));
  }
}
