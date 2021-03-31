#include "value_range.hh"

ValueRange::ValueRange(const ValueVector& values)
  : beginIt(values.begin()),
    endIt(values.end())
{
  assert(begin() <= end());
}

ValueRange::ValueRange(ValueIterator beginIt,
                       ValueIterator endIt)
  : beginIt(beginIt),
    endIt(endIt)
{
  assert(begin() <= end());
}

ValueRange::ValueRange(ReverseValueIterator rbegin,
                       ReverseValueIterator rend)
  : beginIt(ValueIterator(rend.base())),
    endIt(ValueIterator(rbegin.base()))
{
  assert(begin() <= end());
}

std::pair<ValueRange, ValueRange> ValueRange::split(ValueIterator middle)
{
  assert(begin() <= middle);
  assert(middle <= end());

  auto next = middle;
  ++next;

  return std::make_pair(ValueRange(begin(), middle),
                        ValueRange(next, end()));
}


ValueRange ValueRange::innerRange() const
{
  assert(!isEmpty());

  auto next = begin();
  ++next;
  auto prev = end();
  --prev;

  return ValueRange(next, prev);
}

idx ValueRange::getSize() const
{
  return std::distance(begin(), end());
}

bool ValueRange::isEmpty() const
{
  return begin() == end();
}

ValueIterator ValueRange::middle() const
{
  assert(!isEmpty());
  ValueIterator it = begin();
  std::advance(it, getSize() / 2);

  assert(it >= begin());
  assert(it < end());

  return it;
}
