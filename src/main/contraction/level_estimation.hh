#ifndef LEVEL_ESTIMATION_HH
#define LEVEL_ESTIMATION_HH

#include "graph/vertex_map.hh"

#include "contraction_preprocessor.hh"
#include "score_function.hh"

class LevelEstimation : public ScoreFunction
{
private:
  const Graph& overlayGraph;
  VertexMap<idx> levelEstimation;

public:
  LevelEstimation(const Graph& overlayGraph);

  virtual float getScore(Vertex vertex,
                         const std::vector<ContractionPair>& pairs) override;

  void vertexContracted(Vertex vertex,
                        const std::vector<ContractionResult>& results) override;

};

#endif /* LEVEL_ESTIMATION_HH */
