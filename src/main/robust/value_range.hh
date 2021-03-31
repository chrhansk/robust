#ifndef VALUE_RANGE_HH
#define VALUE_RANGE_HH

#include <cassert>

#include "robust/robust_utils.hh"

class ValueRange
{
private:
  ValueIterator beginIt, endIt;

public:
  ValueRange(const ValueVector& values);

  ValueRange(ValueIterator begin,
             ValueIterator end);

  ValueRange(ReverseValueIterator rbegin,
             ReverseValueIterator rend);

  bool isEmpty() const;

  idx getSize() const;

  ValueIterator begin() const
  {
    return beginIt;
  }

  ValueIterator end() const
  {
    return endIt;
  }

  ReverseValueIterator rbegin() const
  {
    return ReverseValueIterator(endIt);
  }

  ReverseValueIterator rend() const
  {
    return ReverseValueIterator(beginIt);
  }

  ValueIterator middle() const;

  num getFirst() const
  {
    return *(begin());
  }

  num getLast() const
  {
    return *(ReverseValueIterator(end()));
  }

  ValueRange innerRange() const;

  std::pair<ValueRange, ValueRange> split(ValueIterator middle);
};

#endif /* VALUE_RANGE_HH */
