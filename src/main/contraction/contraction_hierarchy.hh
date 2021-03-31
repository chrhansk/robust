#ifndef CONTRACTION_HIERARCHY_HH
#define CONTRACTION_HIERARCHY_HH

#include "graph/graph.hh"
#include "graph/edge_map.hh"
#include "graph/vertex_map.hh"

#include "router/router.hh"

#include "edge_pair.hh"

class ContractionEdge
{
public:
  ContractionEdge()
  {}
  ContractionEdge(Vertex vertex, Edge edge, num cost)
    : vertex(vertex),
      edge(edge),
      cost(cost)
  {}
  Vertex vertex;
  Edge edge;
  num cost;
};

class ContractionHierarchy
{
private:
  Graph graph;
  VertexMap<Vertex> permutation;
  VertexMap<std::vector<ContractionEdge>> upwardEdges, downwardEdges;
  EdgeMap<EdgePair> originalEdges;

  Path unpack(const Path& overlayPath) const;

public:
  ContractionHierarchy(const Graph& overlayGraph,
                       const EdgeFunc<num>& overlayCosts,
                       const VertexMap<num>& ranks,
                       const EdgeFunc<EdgePair>& edgePairs);

  class Router : public ::Router
  {
  private:
    template <bool bounded>
    SearchResult findShortestPath(Vertex source,
                                  Vertex target,
                                  num bound = inf);

    const ContractionHierarchy& hierarchy;
  public:
    Router(const ContractionHierarchy& hierarchy)
      : hierarchy(hierarchy)
    {}


    SearchResult shortestPath(Vertex source,
                              Vertex target,
                              const EdgeFunc<num>& costs) override;

    SearchResult shortestPath(Vertex source,
                              Vertex target,
                              const EdgeFunc<num>& costs,
                              num bound) override;
  };

  Router getRouter() const
  {
    return Router(*this);
  }
};


#endif /* CONTRACTION_HIERARCHY_HH */
