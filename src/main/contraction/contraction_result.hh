#ifndef CONTRACTION_RESULT_HH
#define CONTRACTION_RESULT_HH

#include "graph/graph.hh"
#include "contraction_pair.hh"

class ContractionResult
{
private:
  const ContractionPair& pair;
  Edge edge;

public:
  ContractionResult(const ContractionPair& pair, Edge edge)
    : pair(pair), edge(edge)
  {
  }

  Edge getEdge() const
  {
    return edge;
  }

  const ContractionPair& getPair() const
  {
    return pair;
  }
};

#endif /* CONTRACTION_RESULT_HH */
