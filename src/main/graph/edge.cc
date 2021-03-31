#include "edge.hh"

#include <cassert>

Vertex Edge::getSource() const
{
  return source;
}

Vertex Edge::getTarget() const
{
  return target;
}

Vertex Edge::getEndpoint(Direction direction) const
{
  return (direction == Direction::OUTGOING)
    ? getTarget()
    : getSource();
}

Vertex Edge::getOpposite(Vertex vertex) const
{
  if(vertex == getSource())
  {
    return getTarget();
  }
  else
  {
    assert(vertex == getTarget());
    return getSource();
  }
}

idx Edge::getIndex() const
{
  return index;
}

bool Edge::operator==(const Edge& other) const
{
  return getIndex() == other.getIndex();
}
