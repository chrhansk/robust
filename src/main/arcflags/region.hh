#ifndef REGION_HH
#define REGION_HH

#include "graph/graph.hh"
#include "util.hh"

/**
 * A region is equivalent to a set of vertices.
 **/
class Region
{
private:
  idx index;
  std::vector<Vertex> vertices;

public:
  Region(idx index, const std::vector<Vertex>& vertices)
    : index(index), vertices(vertices) {}


  idx getIndex() const
  {
    return index;
  }

  std::vector<Vertex>& getVertices()
  {
    return vertices;
  }

  const std::vector<Vertex>& getVertices() const
  {
    return vertices;
  }

  bool operator==(const Region& other) const
  {
    return this == &other;
  }

  bool operator!=(const Region& other) const
  {
    return !(*this == other);
  }
};

namespace std
{
  /**
   * A hash function for edges
   */
  template<> struct hash<Region>
  {
    typedef Region argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& region) const
    {
      return region.getIndex();
    }
  };
}


#endif /* REGION_HH */
