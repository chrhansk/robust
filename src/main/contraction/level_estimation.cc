#include "level_estimation.hh"


LevelEstimation::LevelEstimation(const Graph& overlayGraph)
  : overlayGraph(overlayGraph),
    levelEstimation(overlayGraph, 0)
{}

float LevelEstimation::getScore(Vertex vertex,
                                const std::vector<ContractionPair>& pairs)
{
  return levelEstimation(vertex);
}

void LevelEstimation::vertexContracted(Vertex vertex,
                                       const std::vector<ContractionResult>& results)
{
  idx level = levelEstimation(vertex);

  for(const Edge& edge : overlayGraph.getOutgoing(vertex))
  {
    const Vertex& target = edge.getTarget();

    levelEstimation(target) = std::max(levelEstimation(target),
                                       level + 1);
  }
  for(const Edge& edge : overlayGraph.getIncoming(vertex))
  {
    const Vertex& source = edge.getTarget();

    levelEstimation(source) = std::max(levelEstimation(source),
                                       level + 1);
  }
}
