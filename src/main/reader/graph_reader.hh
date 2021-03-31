#ifndef GRAPH_READER_HH
#define GRAPH_READER_HH

#include <iostream>

#include "graph/graph.hh"
#include "graph/edge_map.hh"
#include "graph/vertex_map.hh"

class ReadResult
{
public:
  ReadResult(const Graph& graph,
             const EdgeMap<num>& costs,
             const EdgeMap<num>& deviations,
             const VertexMap<Point> &points)
    : graph(graph),
      costs(costs),
      deviations(deviations),
      points(points)
  {}

  Graph graph;
  EdgeMap<num> costs;
  EdgeMap<num> deviations;

  VertexMap<Point> points;
};

/**
 * A class to read in a Graph and associated EdgeMap%s from
 * a PBF input stream.
 **/
class GraphReader
{
public:
  ReadResult readGraph(std::istream& in);
};



#endif /* GRAPH_READER_HH */
