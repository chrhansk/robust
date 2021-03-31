#include "partition.hh"

#include <algorithm>
#include <cassert>

Partition::Partition(const Graph& graph)
  : regionIndices(graph, Empty),
    graph(graph)
{

}

Partition::Partition(Partition&& other)
  :regionIndices(std::move(other.regionIndices)),
   graph(other.graph),
   regions(std::move(other.regions))
{

}

bool Partition::isValid() const
{
  VertexSet found(graph);

  for(const Region& region : regions)
  {
    for(const Vertex& vertex : region.getVertices())
    {
      if(found.contains(vertex))
      {
        return false;
      }
      found.insert(vertex);
    }
  }

  for(const Vertex& vertex : graph.getVertices())
  {
    if(!found.contains(vertex))
    {
      return false;
    }
  }

  return true;
}

Region& Partition::addRegion(const std::vector<Vertex>& vertices)
{
  idx index = regions.size();
  regions.push_back(Region(index, vertices));

  for(const Vertex& vertex : vertices)
  {
    regionIndices(vertex) = index;
  }

  return *regions.rbegin();
}

Region& Partition::getRegion(Vertex vertex)
{
  idx index = regionIndices(vertex);

  assert(index != Empty);

  return regions[index];
}

const Region& Partition::getRegion(Vertex vertex) const
{
  idx index = regionIndices(vertex);

  assert(index != Empty);

  return regions[index];
}

std::vector<Region>& Partition::getRegions()
{
  return regions;
}

const std::vector<Region>& Partition::getRegions() const
{
  return regions;
}

std::unordered_set<Edge>
Partition::internalEdges(const Region& region) const
{
  std::unordered_set<Edge> edges;

  for(const Vertex& vertex: region.getVertices())
  {
    for(const Edge& edge : graph.getOutgoing(vertex))
    {
      if(getRegion(edge.getTarget()) == region)
      {
        edges.insert(edge);
      }
    }
  }

  return edges;
}


