#ifndef GEOMETRIC_PARTITION_HH
#define GEOMETRIC_PARTITION_HH

#include "partition.hh"

/**
 * A Partition which is computed by repeatedly
 * bisecting the vertices according to their
 * coordinates.
 **/
class GeometricPartion : public Partition
{
public:
  GeometricPartion(const Graph& graph,
                   const VertexMap<Point>& points,
                   int levels);

private:
  void construct(std::vector<Vertex>::iterator begin,
                 std::vector<Vertex>::iterator end,
                 const VertexMap<Point>& points, int depth);

  int levels;

};

#endif /* GEOMETRIC_PARTITION_HH */
