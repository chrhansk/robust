#ifndef EDGE_COUNT_HH
#define EDGE_COUNT_HH

#include "graph/edge_map.hh"

#include "contraction_preprocessor.hh"
#include "score_function.hh"

class EdgeCount : public ScoreFunction
{
private:
  const Graph& overlayGraph;
  EdgeMap<idx> edgeCount;

public:
  EdgeCount(const Graph& overlayGraph);

  virtual float getScore(Vertex vertex,
                         const std::vector<ContractionPair>& pairs) override;

  void vertexContracted(Vertex vertex,
                        const std::vector<ContractionResult>& results) override;

};


#endif /* EDGE_COUNT_HH */
