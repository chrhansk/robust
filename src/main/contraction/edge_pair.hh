#ifndef EDGE_PAIR_HH
#define EDGE_PAIR_HH

#include "graph/graph.hh"

class EdgePair
{
public:
  EdgePair(const Edge& first, const Edge& second)
    : first(first),
      second(second)
  {}
  EdgePair(const Edge& edge)
    : first(edge)
  {}
  EdgePair()
  {}

  Edge first, second;
};

#endif /* EDGE_PAIR_HH */
