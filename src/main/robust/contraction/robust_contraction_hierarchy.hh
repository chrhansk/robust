#ifndef ROBUST_CONTRACTION_HIERARCHY_HH
#define ROBUST_CONTRACTION_HIERARCHY_HH

#include "graph/graph.hh"
#include "router/router.hh"

#include "contraction/edge_pair.hh"

#include "robust/theta/theta_router.hh"
#include "contraction_range.hh"


class RobustContractionEdge
{
private:
  Vertex vertex;
  Edge edge;
  ContractionRange range;

public:
  RobustContractionEdge()
  {}
  RobustContractionEdge(Vertex vertex, Edge edge, const ContractionRange& range)
    : vertex(vertex),
      edge(edge),
      range(range)
  {}

  const Edge& getEdge() const
  {
    return edge;
  }

  const ContractionRange& getRange() const
  {
    return range;
  }

  const Vertex& getVertex() const
  {
    return vertex;
  }
};

class RobustContractionHierarchy
{
private:
  Graph graph;
  VertexMap<Vertex> permutation;
  VertexMap<std::vector<RobustContractionEdge>> upwardEdges, downwardEdges;
  EdgeMap<EdgePair> originalEdges;

  Path unpack(const Path& overlayPath) const;

public:
  RobustContractionHierarchy(const Graph& overlayGraph,
                             const EdgeFunc<const ContractionRange&>& contractionRanges,
                             const VertexMap<num>& ranks,
                             const EdgeFunc<const EdgePair&>& originalEdges);

  class Router : public ThetaRouter
  {
  private:
    template <bool bounded>
    SearchResult findShortestPath(Vertex source,
                                  Vertex target,
                                  num theta,
                                  num bound = inf);

    const RobustContractionHierarchy& hierarchy;
  public:
    Router(const RobustContractionHierarchy& hierarchy)
      : hierarchy(hierarchy)
    {}


    SearchResult shortestPath(Vertex source,
                              Vertex target,
                              num theta) override;

    SearchResult shortestPath(Vertex source,
                              Vertex target,
                              num theta,
                              num bound) override;
  };

  Router getRouter() const
  {
    return Router(*this);
  }
};


#endif /* ROBUST_CONTRACTION_HIERARCHY_HH */
