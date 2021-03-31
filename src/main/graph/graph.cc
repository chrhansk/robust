#include "graph.hh"

#include <cassert>

#include "vertex_set.hh"

Graph::Graph(idx size, const std::vector<Edge>& edges)
  :size(size), edges(edges)
{
  idx i = 0;

  for(idx i = 0; i < size; ++i)
  {
    outgoing.push_back(std::vector<Edge>());
    incoming.push_back(std::vector<Edge>());
  }

  i = 0;

  for(const Edge& edge : edges)
  {
    assert(edge.getIndex() == i);
    assert(edge.getSource().getIndex() < size);
    assert(edge.getTarget().getIndex() < size);
    i++;
    outgoing[edge.getSource().getIndex()].push_back(edge);
    incoming[edge.getTarget().getIndex()].push_back(edge);
  }

  assert(check());
}

const std::vector<Edge>& Graph::getEdges() const
{
  return edges;
}

Vertices Graph::getVertices() const
{
  return Vertices(size);
}

const std::vector<Edge>& Graph::getOutgoing(Vertex vertex) const
{
  return outgoing[vertex.getIndex()];
}

const std::vector<Edge>& Graph::getIncoming(Vertex vertex) const
{
  return incoming[vertex.getIndex()];
}

const std::vector<Edge>& Graph::getEdges(Vertex vertex,
                                         Direction direction) const
{
  return (direction == Direction::OUTGOING) ?
    getOutgoing(vertex) :
    getIncoming(vertex);
}

bool Graph::contains(const Edge& edge) const
{
  for(const Edge& outEdge : getOutgoing(edge.getSource()))
  {
    if(outEdge == edge)
    {
      return true;
    }
  }

  return false;
}

Edge Graph::addEdge(Vertex source, Vertex target)
{
  assert((size_t) source.getIndex() < getVertices().size());
  assert((size_t) target.getIndex() < getVertices().size());

  Edge edge(source, target, edges.size());
  edges.push_back(edge);
  outgoing[source.getIndex()].push_back(edge);
  incoming[target.getIndex()].push_back(edge);

  assert(check());

  return edge;
}

bool Graph::check() const
{
  for(const Vertex& vertex : getVertices())
  {
    for(const Edge& edge : getOutgoing(vertex))
    {
      assert(edge.getSource() == vertex);
    }

    for(const Edge& edge : getIncoming(vertex))
    {
      assert(edge.getTarget() == vertex);
    }
  }

  for(uint j = 0; j < getEdges().size(); ++j)
  {
    const Edge& edge = getEdges()[j];
    assert(edge.getIndex() == j);

    const Vertex& source = edge.getSource();
    const Vertex& target = edge.getTarget();

    bool found = false;

    for(const Edge& outEdge : getOutgoing(source))
    {
      if(outEdge == edge)
      {
        assert(outEdge.getSource() == source);
        assert(outEdge.getTarget() == target);
        assert(!found);
        found = true;
      }
    }

    assert(found);
    found = false;

    for(const Edge& inEdge : getIncoming(target))
    {
      if(inEdge == edge)
      {
        assert(inEdge.getSource() == source);
        assert(inEdge.getTarget() == target);
        assert(!found);
        found = true;
      }
    }

    assert(found);
  }

  return true;
}
