#include "search_interval.hh"


SearchInterval::SearchInterval(ValueInterval values,
                               num leftPathCost,
                               num rightPathCost,
                               idx deviationSize)
  : values(values),
    leftPathCost(leftPathCost),
    rightPathCost(rightPathCost),
    deviationSize(deviationSize),
    lowerBoundCost(getLowerBound(deviationSize))
{
  assert(leftPathCost <= rightPathCost);
}

SearchInterval::SearchInterval(const ValueVector& valueVector,
                               num leftPathCost,
                               num rightPathCost,
                               idx deviationSize)
  : values(valueVector),
    leftPathCost(leftPathCost),
    rightPathCost(rightPathCost),
    deviationSize(deviationSize),
    lowerBoundCost(getLowerBound(deviationSize))
{
}

SearchInterval SearchInterval::leftInterval(const SearchInterval& other,
                                            ValueIterator middle,
                                            num value)
{
  return SearchInterval(ValueInterval(other.getValues().begin(), middle),
                        other.leftPathCost,
                        std::min(value, other.rightPathCost),
                        other.deviationSize);
}

SearchInterval SearchInterval::rightInterval(const SearchInterval& other,
                                             ValueIterator middle,
                                             num value)
{
  return SearchInterval(ValueInterval(middle, other.getValues().end()),
                        std::min(value, other.leftPathCost),
                        other.rightPathCost,
                        other.deviationSize);
}

num SearchInterval::getLowerBound(idx deviationSize)
{
  num minValue = values.last();
  return leftPathCost + deviationSize * minValue;
}

bool SearchInterval::tighten(num bestCost, int deviationSize)
{
  ValueIterator it;

  int size = getValues().size();

  int shrink = 0;

  for(it = getValues().begin(); it != getValues().end(); ++it)
  {
    if(deviationSize*(*it) <= bestCost - rightPathCost)
    {
      break;
    }

    ++shrink;
  }

  if(shrink == 0)
  {
    return false;
  }

  int newSize = std::distance(it, values.end());

  assert(newSize + shrink == size);

  values = ValueInterval(values.begin(), it);

  return true;
}
