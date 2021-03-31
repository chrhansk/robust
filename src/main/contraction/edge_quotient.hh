#ifndef EDGE_QUOTIENT_HH
#define EDGE_QUOTIENT_HH

#include "score_function.hh"

#include "contraction_preprocessor.hh"
#include "witness_path_search.hh"

class EdgeQuotient : public ScoreFunction
{
private:
  const Graph& overlayGraph;

public:
  EdgeQuotient(const Graph& overlayGraph)
    : overlayGraph(overlayGraph)
  {}

  float getScore(Vertex vertex,
                 const std::vector<ContractionPair>& pairs) override;
};

#endif /* EDGE_QUOTIENT_HH */
