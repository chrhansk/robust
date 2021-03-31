#include "geometric_partition.hh"

#include <algorithm>
#include <iostream>
#include <cassert>

GeometricPartion::GeometricPartion(const Graph& graph,
                                   const VertexMap<Point>& points,
                                   int levels)
  : Partition(graph), levels(levels)
{
  std::vector<Vertex> vertices = graph.getVertices().collect();

  construct(vertices.begin(),
            vertices.end(),
            points, 0);

  assert(isValid());
}

void GeometricPartion::construct(std::vector<Vertex>::iterator begin,
                                 std::vector<Vertex>::iterator end,
                                 const VertexMap<Point>& points,
                                 int depth)
{
  int dist = std::distance(begin, end);

  if(dist == 0)
  {
    return;
  }

  if(depth == levels or dist <= 1)
  {
    std::vector<Vertex> vertices(begin, end);
    addRegion(vertices);
    return;
  }

  if(depth % 2 == 0)
  {
    std::sort(begin, end,
              [&points](const Vertex& first, const Vertex& second)
              {
                return points(first).getX() < points(second).getX();
              }
      );
  }
  else
  {
    std::sort(begin, end,
              [&points](const Vertex& first, const Vertex& second)
              {
                return points(first).getX() < points(second).getX();
              }
      );
  }

  auto middle = begin;
  std::advance(middle, dist / 2);

  construct(begin, middle, points, depth + 1);
  construct(middle, end, points, depth + 1);

}
