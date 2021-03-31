#ifndef ABSTRACT_CONTRACTION_PREPROCESSOR_HH
#define ABSTRACT_CONTRACTION_PREPROCESSOR_HH

#include <boost/heap/d_ary_heap.hpp>

#include "graph/graph.hh"
#include "graph/edge_map.hh"
#include "graph/vertex_map.hh"

#include "contraction_pair.hh"
#include "contraction_result.hh"
#include "contraction_hierarchy.hh"

class AbstractContractionPreprocessor
{
public:
  AbstractContractionPreprocessor(const Graph& graph,
                                  const EdgeFunc<num>& costs)
    : graph(graph),
      costs(costs)
  {}

  virtual ContractionHierarchy computeHierarchy() = 0;

protected:

  typedef typename boost::heap::d_ary_heap<Vertex,
                                           boost::heap::mutable_<true>,
                                           boost::heap::compare<VertexCompare<float,
                                                                              true>>,
                                           boost::heap::arity<2>> Queue;

  typedef typename Queue::handle_type Handle;

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

  std::vector<ContractionResult>
  contractVertex(const Vertex& vertex,
                 Graph& overlayGraph,
                 EdgeMap<num>& overlayCosts,
                 EdgeMap<EdgePair>& originalEdges,
                 const VertexFunc<bool>& contracted,
                 const std::vector<ContractionPair>& pairs);
};


#endif /* ABSTRACT_CONTRACTION_PREPROCESSOR_HH */
