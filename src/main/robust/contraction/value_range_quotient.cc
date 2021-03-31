#include "value_range_quotient.hh"

float ValueRangeQuotient::getScore(Vertex vertex,
                                   const std::vector<RobustContractionPair>& pairs)
{
  num sumAdded = 0, sumRemoved = 0;

  for(const RobustContractionPair& pair : pairs)
  {
    sumAdded += pair.valueCount();
  }

  for(const Edge& incoming : graph.getIncoming(vertex))
  {
    sumRemoved += contractionRanges(incoming).rangeSize();
  }

  for(const Edge& outgoing : graph.getOutgoing(vertex))
  {
    sumRemoved += contractionRanges(outgoing).rangeSize();
  }

  if(sumRemoved == 0)
  {
    return 0;
  }

  float value = sumAdded / ((float) sumRemoved);

  return value;
}
