#include "simple_active_router.hh"

#include "robust/reduced_costs.hh"

ActiveRouter::ActiveSearchResult
SimpleActiveRouter::findShortestPath(Vertex source,
                                     Vertex target,
                                     num value)
{
  ReducedCosts reducedCosts(costs, deviations, value);

  LabelHeap<Label> heap(graph);
  int settled = 0, labeled = 0;
  bool found = false;

  heap.update(Label(source, Edge(), 0));

  while(!heap.isEmpty())
  {
    const Label& current = heap.extractMin();

    ++settled;

    if(current.getVertex() == target)
    {
      found = true;
      break;
    }

    for(const Edge& edge : graph.getOutgoing(current.getVertex()))
    {
      ++labeled;

      Label nextLabel = Label(edge.getTarget(),
                              edge, current.getCost() + reducedCosts(edge));

      heap.update(nextLabel);
    }
  }

  if(found)
  {
    Path path;

    Label current = heap.getLabel(target);

    while(!(current.getVertex() == source))
    {
      Edge edge = current.getEdge();
      path.prepend(edge);
      current = heap.getLabel(edge.getSource());
    }

    ActiveSearchResult searchResult(SearchResult(settled, labeled, found, path, current.getCost()));

    for(const Vertex& vertex : graph.getVertices())
    {
      if(heap.getLabel(vertex).getState() != State::SETTLED)
      {
        continue;
      }

      std::vector<Edge> edges;

      std::copy_if(graph.getOutgoing(vertex).begin(),
                   graph.getOutgoing(vertex).end(),
                   std::back_inserter(edges),
                   [&](const Edge& edge)
                   {
                     return deviations(edge) > value;
                   });

      if(not(edges.empty()))
      {
        searchResult.getActiveEdges().push_back(*std::min_element(edges.begin(),
                                                                  edges.end(),
                                                                  [&](const Edge& first, const Edge& second)
                                                                  {
                                                                    return deviations(first) < deviations(second);
                                                                  }));
      }

    }

    return searchResult;
  }

  return ActiveSearchResult(SearchResult::notFound(settled, labeled));
}
