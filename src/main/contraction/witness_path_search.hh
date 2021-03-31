#ifndef WITNESS_PATH_SEARCH_HH
#define WITNESS_PATH_SEARCH_HH

#include <vector>

#include "graph/graph.hh"
#include "graph/vertex_map.hh"
#include "path/path.hh"

#include "contraction_pair.hh"

class SearchPredicate
{
private:
  const VertexFunc<bool>& contracted;
  Vertex vertex;
public:
  SearchPredicate(const VertexFunc<bool>& contracted,
                  Vertex vertex)
    : contracted(contracted),
      vertex(vertex)
  {}

  bool operator()(const Edge& edge)
  {
    const Vertex& source = edge.getSource();
    const Vertex& target = edge.getTarget();

    return !contracted(source) and
      !contracted(target) and
      source != vertex and
      target != vertex;
  }
};

/**
 * The base class for all witness path searches.
 **/
class WitnessPathSearch
{
protected:
  const Graph& graph;
  const EdgeFunc<num>& costs;
  const VertexFunc<bool>& contracted;

public:
  WitnessPathSearch(const Graph& graph,
                    const EdgeFunc<num>& costs,
                    const VertexFunc<bool>& contracted)
    : graph(graph),
      costs(costs),
      contracted(contracted)
  {}

  /**
   * Returns all ContractionPair%s for the given Vertex. When
   * the given Vertex is contracted, then Edge%s between all
   * the returned ContracionPair%s will be added.
   **/
  virtual std::vector<ContractionPair> findPairs(Vertex vertex) const = 0;
};


#endif /* WITNESS_PATH_SEARCH_HH */
