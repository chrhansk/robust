#ifndef NESTED_DISSECTION_ORDER_HH
#define NESTED_DISSECTION_ORDER_HH

#include "graph/graph.hh"
#include "graph/vertex_map.hh"

class NestedDissectionOrder : public VertexMap<num>
{
public:
  NestedDissectionOrder(const Graph& graph);
private:
  bool check(const Graph& graph) const;
};

#endif /* NESTED_DISSECTION_ORDER_HH */
