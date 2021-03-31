#include "size_quotient.hh"


float SizeQuotient::getScore(Vertex vertex,
                             const std::vector<RobustContractionPair>& pairs)
{
  num sumAdded = 0, sumRemoved = 0;
  const Graph& graph = preprocessor.getOverlayGraph();
  const EdgeMap<ContractionRange>& contractionRanges = preprocessor.getContractionRanges();

  for(const RobustContractionPair& pair : pairs)
  {
    sumAdded += pair.valueCount();
  }

  for(const Edge& incoming : graph.getIncoming(vertex))
  {
    sumRemoved += contractionRanges(incoming).valueCount();
  }

  for(const Edge& outgoing : graph.getOutgoing(vertex))
  {
    sumRemoved += contractionRanges(outgoing).valueCount();
  }

  if(sumRemoved == 0)
  {
    return 0;
  }

  float value = sumAdded / ((float) sumRemoved);

  return value;
}
