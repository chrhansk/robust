#include "edge_count.hh"


EdgeCount::EdgeCount(const Graph& overlayGraph)
  : overlayGraph(overlayGraph),
    edgeCount(overlayGraph, 1)
{

}

float EdgeCount::getScore(Vertex vertex,
                          const std::vector<ContractionPair>& pairs)
{
  idx removedSum = 0, addedSum = 0;

  for(const Edge& edge : overlayGraph.getIncoming(vertex))
  {
    removedSum += edgeCount(edge);
  }

  for(const Edge& edge : overlayGraph.getOutgoing(vertex))
  {
    removedSum += edgeCount(edge);
  }

  if(removedSum == 0)
  {
    return 0;
  }

  for(const ContractionPair& pair : pairs)
  {
    addedSum += pair.getDefaultPath().cost(edgeCount.getValues());
  }

  return addedSum / removedSum;
}

void EdgeCount::vertexContracted(Vertex vertex,
                                 const std::vector<ContractionResult>& results)
{
  for(const ContractionResult& result : results)
  {
    edgeCount.extend(result.getEdge(), 1);
  }

  for(const ContractionResult& result : results)
  {
    edgeCount(result.getEdge()) =
      result.getPair().getDefaultPath().cost(edgeCount.getValues());
  }
}
