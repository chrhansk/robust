#ifndef PARTITION_HH
#define PARTITION_HH

#include <unordered_set>

#include "graph/graph.hh"
#include "graph/vertex_map.hh"
#include "util.hh"

#include "region.hh"

class Boundary
{
private:
  const Region& region;
  std::vector<Vertex> vertices;

public:
  Boundary(const Region& region, const std::vector<Vertex>& vertices)
    : region(region),
      vertices(vertices)
  {}

  const Region& getRegion() const
  {
    return region;
  }

  const std::vector<Vertex>& getVertices() const
  {
    return vertices;
  }

};

/**
 * A partitioning of vertices into Region%s.
 **/
class Partition
{
private:
  VertexMap<idx> regionIndices;
  const Graph& graph;
  std::vector<Region> regions;

  static const idx Empty = -1;

public:
  /**
   * Creates an empty Partition.
   **/
  Partition(const Graph& graph);

  Partition(Partition&& other);

  Partition(const Partition& other) = delete;
  Partition& operator=(const Partition& other) = delete;

  const Graph& getGraph() const
  {
    return graph;
  }

  bool operator==(const Partition& other) const
  {
    return this == &other;
  }

  /**
   * Adds the given vertices to the Partition as a new Region.
   * @return the new Region
   **/
  Region& addRegion(const std::vector<Vertex>& vertices);

  /**
   * Returns whether this Partition actually assigns each
   * Vertex to exactly one Region.
   **/
  bool isValid() const;

  /**
   * Returns the Region containing the given Vertex.
   **/
  Region& getRegion(Vertex vertex);

  /**
   * Returns the Region containing the given Vertex.
   **/
  const Region& getRegion(Vertex vertex) const;

  /**
   * Return all Region%s in this Partition.
   **/
  std::vector<Region>& getRegions();

  /**
   * Return all Region%s in this Partition.
   **/
  const std::vector<Region>& getRegions() const;

  /**
   * A filter which evaluates to true on all vertices
   * belonging to a given Region and on all Edges%s entirely
   * within a Region.
   **/
  class RegionFilter
  {
  private:
    const Partition& partition;
    const Region& region;

  public:
    RegionFilter(const Partition& partition,
                 const Region& region)
      : partition(partition),
        region(region)
    {}

    bool operator()(const Vertex& vertex) const
    {
      return partition.getRegion(vertex) == region;
    }

    bool operator()(const Edge& edge) const
    {
      return partition.getRegion(edge.getSource()) == region and
        partition.getRegion(edge.getTarget()) == region;
    }
  };

  /**
   * Returns a RegionFilter for the given Region.
   **/
  RegionFilter regionFilter(const Region& region) const
  {
    return RegionFilter(*this, region);
  }

  template <Direction direction>
  std::unordered_set<Vertex> boundaryVertices(const Region& region) const;

  template <Direction direction>
  std::unordered_set<Edge> boundaryEdges(const Region& region) const;

  std::unordered_set<Edge> internalEdges(const Region& region) const;

  template <Direction direction>
  Boundary getBoundary(const Region& region) const;
};

template <Direction direction>
std::unordered_set<Vertex>
Partition::boundaryVertices(const Region& region) const
{
  std::unordered_set<Vertex> vertices;

  for(const Vertex& vertex: region.getVertices())
  {
    for(const Edge& edge : graph.getEdges(vertex, direction))
    {
      if(getRegion(edge.getEndpoint(direction)) != region)
      {
        vertices.insert(vertex);
      }
    }
  }

  return vertices;
}

template <Direction direction>
std::unordered_set<Edge>
Partition::boundaryEdges(const Region& region) const
{
  std::unordered_set<Edge> edges;

  for(const Vertex& vertex: region.getVertices())
  {
    for(const Edge& edge : graph.getEdges(vertex, direction))
    {
      if(getRegion(edge.getEndpoint(direction)) != region)
      {
        edges.insert(edge);
      }
    }
  }

  return edges;
}

template <Direction direction>
Boundary Partition::getBoundary(const Region& region) const
{
  std::vector<Vertex> vertices;

  for(const Vertex& vertex: region.getVertices())
  {
    for(const Edge& edge : graph.getEdges(vertex, direction))
    {
      if(getRegion(edge.getEndpoint(direction)) != region)
      {
        vertices.push_back(vertex);
      }
    }
  }

  return Boundary(region, vertices);
}

#endif /* PARTITION_HH */
