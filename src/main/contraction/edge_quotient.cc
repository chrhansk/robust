#include "edge_quotient.hh"

float EdgeQuotient::getScore(Vertex vertex,
                             const std::vector<ContractionPair>& pairs)
{
  const num numAdded = pairs.size();
  num numRemoved = overlayGraph.getIncoming(vertex).size() +
    overlayGraph.getOutgoing(vertex).size();

  float value = numAdded / (float (numRemoved));

  return value;
}
