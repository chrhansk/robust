#ifndef SEARCH_INTERVAL_HH
#define SEARCH_INTERVAL_HH

#include "robust_utils.hh"

class SearchInterval
{
private:
  num getLowerBound(idx deviationSize);

  ValueInterval values;

public:
  SearchInterval(ValueInterval values,
                 num leftPathCost,
                 num rightPathCost,
                 idx deviationSize);

  SearchInterval(const ValueVector& values,
                 num leftPathCost,
                 num rightPathCost,
                 idx deviationSize);

  // the cost of the shortest path with respect to *begin
  num leftPathCost;

  // the cost of the shortest path with respect to *rbegin
  num rightPathCost;

  idx deviationSize;

  // a lower bound on the objective which can be
  // attained in this interval
  num lowerBoundCost;

  static SearchInterval leftInterval(const SearchInterval& interval,
                                     ValueIterator middle,
                                     num value);

  static SearchInterval rightInterval(const SearchInterval& interval,
                                      ValueIterator middle,
                                      num value);

  bool tighten(num bestCost, int deviationSize);

  const ValueInterval& getValues() const
  {
    return values;
  }

  ValueInterval& getValues()
  {
    return values;
  }

};

#endif /* SEARCH_INTERVAL_HH */
