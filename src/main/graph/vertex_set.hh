#ifndef VERTEX_SET_HH
#define VERTEX_SET_HH

#include "graph/graph.hh"

/**
 * A set of vertices. Realized as a vector of
 * boolean values.
 **/
class VertexSet
{
private:
  std::vector<bool> values;
public:
  /**
   * Creates a new (empty) VertexSet
   **/
  VertexSet(const Graph& graph)
    : values(graph.getVertices().size(), false)
  {}

  /**
   * Returns whether the given Vertex is contained
   * in this VertexSet.
   **/
  bool contains(const Vertex& vertex) const
  {
    return values[vertex.getIndex()];
  }

  /**
   * Inserts a given Vertex into this VertexSet.
   **/
  void insert(const Vertex& vertex)
  {
    values[vertex.getIndex()] = true;
  }

  /**
   * Removes a given Vertex from this VertexSet.
   **/
  void remove(const Vertex& vertex)
  {
    values[vertex.getIndex()] = false;
  }
};

#endif /* VERTEX_SET_HH */
