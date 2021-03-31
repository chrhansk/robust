#ifndef ABSTRACT_ROBUST_CONTRACTION_PREPROCESSOR_HH
#define ABSTRACT_ROBUST_CONTRACTION_PREPROCESSOR_HH

#include "graph/graph.hh"
#include "graph/edge_map.hh"
#include "graph/vertex_map.hh"

#include "robust/robust_utils.hh"

#include "robust_contraction_hierarchy.hh"
#include "robust_contraction_pair.hh"

class AbstractRobustContractionPreprocessor
{
protected:
  static const num INVALID;

  class Contracted : public VertexFunc<bool>
  {
  private:
    const VertexMap<num>& rankMap;

  public:
    Contracted(const VertexMap<num>& rankMap)
      : rankMap(rankMap)
    {
    }

    bool operator()(const Vertex& vertex) const override
    {
      return rankMap(vertex) != INVALID;
    }
  };

  const Graph& graph;
  const EdgeFunc<num>& costs;
  const EdgeFunc<num>& deviations;
  const ValueVector values;

  void removeOverflow(const Graph& overlayGraph,
                      EdgeMap<ContractionRange>& contractionRanges,
                      const Edge& edge) const;


  void removeOverflows(const Graph& overlayGraph,
                       EdgeMap<ContractionRange>& contractionRanges) const;

  void tightenEdges(const Graph& graph,
                    EdgeMap<ContractionRange>& contractionRanges) const;

  void contractVertex(const Vertex& vertex,
                      Graph& overlayGraph,
                      EdgeMap<EdgePair>& originalEdges,
                      EdgeMap<ContractionRange>& contractionRanges,
                      const VertexFunc<bool>& contracted,
                      const std::vector<RobustContractionPair>& pairs) const;

public:
  AbstractRobustContractionPreprocessor(const Graph& graph,
                                        const EdgeFunc<num>& costs,
                                        const EdgeFunc<num>& deviations)
    : graph(graph),
      costs(costs),
      deviations(deviations),
      values(thetaValues(graph, deviations))
  {}

  virtual RobustContractionHierarchy computeHierarchy() const = 0;
};


#endif /* ABSTRAC_ROBUST_CONTRACTION_PREPROCESSOR_HH */
