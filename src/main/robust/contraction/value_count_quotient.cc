#include "value_count_quotient.hh"


float ValueCountQuotient::getScore(Vertex vertex,
                                   const std::vector<RobustContractionPair>& pairs)
{
  num sumAdded = 0, sumRemoved = 0;

  for(const RobustContractionPair& pair : pairs)
  {
    num lowerBound = pair.getMinimum();
    num upperBound = pair.getMaximum();

    assert(lowerBound <= upperBound);

    for(const Edge& edge : pair.getDefaultPath().getEdges())
    {
      const ContractionRange& contractionRange
        = contractionRanges(edge);

      for(num value : contractionRange.getValues())
      {
        if(value > lowerBound and
           value < upperBound)
        {
          ++sumAdded;
        }
      }
    }
  }

  for(const Edge& incoming : graph.getIncoming(vertex))
  {
    sumRemoved += contractionRanges(incoming).getValues().size();
  }

  for(const Edge& outgoing : graph.getOutgoing(vertex))
  {
    sumRemoved += contractionRanges(outgoing).getValues().size();
  }

  if(sumRemoved == 0)
  {
    return 0;
  }

  float value = sumAdded / ((float) sumRemoved);

  return value;
}
