#include "vertex.hh"

idx Vertex::getIndex() const
{
  return index;
}

bool Vertex::operator==(const Vertex& other) const
{
  return getIndex() == other.getIndex();
}

bool Vertex::operator!=(const Vertex& other) const
{
  return !(*this == other);
}

bool Vertex::operator<(const Vertex& other) const
{
  return getIndex() < other.getIndex();
}

bool Vertex::operator<=(const Vertex& other) const
{
  return getIndex() <= other.getIndex();
}

std::ostream& operator<<(std::ostream& out, const Vertex& vertex)
{
  out << vertex.getIndex();
  return out;
}
