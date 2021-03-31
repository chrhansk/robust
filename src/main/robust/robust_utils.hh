#ifndef ROBUST_UTILS_HH
#define ROBUST_UTILS_HH

#include <vector>
#include <unordered_set>

#include "util.hh"

#include "graph/graph.hh"
#include "graph/edge_map.hh"

extern template class std::vector<num>;

typedef std::vector<num> ValueVector;
typedef std::unordered_set<num> ValueSet;

extern template class std::vector<ValueVector>;

/**
 * Determines the required theta values. The values
 * are returned as a ValueVector containing unique
 * values sorted strictly descending. In addition,
 * the value "zero" is added if necessary.
 *
 * @param graph       A graph
 * @param deviations  A map containing deviation values
 *
 * @return A ValueVector containing the required theta values.
 */
ValueVector thetaValues(const Graph& graph,
                        const EdgeFunc<num>& deviations);

typedef ValueVector::const_iterator ValueIterator;

typedef std::reverse_iterator<ValueIterator> ReverseValueIterator;

class ValueInterval
{
private:
  ValueIterator beginIt, endIt;

public:
  ValueInterval(ValueIterator begin, ValueIterator end)
    : beginIt(begin),
      endIt(end)
  {
    assert(beginIt <= endIt);
  }

  ValueInterval(const ValueVector& values)
    : beginIt(values.begin()),
      endIt(values.end())
  {
    assert(beginIt <= endIt);
  }

  ValueIterator begin() const
  {
    return beginIt;
  }

  ValueIterator end() const
  {
    return endIt;
  }

  num first() const
  {
    return *beginIt;
  }

  num last() const
  {
    return *ReverseValueIterator(endIt);
  }

  ReverseValueIterator rbegin() const
  {
    return ReverseValueIterator(endIt);
  }

  ReverseValueIterator rend() const
  {
    return ReverseValueIterator(beginIt);
  }

  template<class Func>
  bool trimBegin(Func func)
  {
    bool trimmed = false;

    while(beginIt != endIt and func(*beginIt))
    {
      trimmed = true;
      ++beginIt;
    }

    return trimmed;
  }

  template<class Func>
  bool trimEnd(Func func)
  {
    ReverseValueIterator rit = rbegin();
    ReverseValueIterator re = rend();
    bool trimmed = false;

    while(rit != re and func(*rit))
    {
      trimmed = true;
      ++rit;
    }

    if(trimmed)
    {
      endIt = rit.base();
    }

    return trimmed;
  }

  std::pair<ValueInterval, ValueInterval> split(idx distance) const
  {
    ValueIterator curr = beginIt;
    std::advance(curr, distance);

    return split(curr);
  }

  std::pair<ValueInterval, ValueInterval> split(ValueIterator curr) const
  {
    assert(curr < endIt);

    ValueIterator next = curr;
    ++next;

    return std::make_pair(ValueInterval(beginIt, curr),
                          ValueInterval(next, endIt));
  }

  ValueIterator middle() const
  {
    const idx distance = size() / 2;

    ValueIterator curr = beginIt;
    std::advance(curr, distance);

    return curr;
  }

  idx size() const
  {
    return std::distance(beginIt, endIt);
  }

  bool isEmpty() const
  {
    return beginIt == endIt;
  }

};


#endif /* ROBUST_UTILS_HH */
