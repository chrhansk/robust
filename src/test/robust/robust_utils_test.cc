#include <gtest/gtest.h>

#include <algorithm>

#include "robust/robust_utils.hh"


TEST(RobustUtilsTest, IterationTest) {
  ValueVector values{10, 9, 6, 4, 3};

  ValueIterator begin = values.begin(), end = begin;

  std::advance(end, 3);

  ASSERT_EQ(3, std::distance(begin, end));

  {
    ReverseValueIterator rbegin(end);
    ReverseValueIterator rend(begin);

    ASSERT_EQ(3, std::distance(rbegin, rend));

    ValueVector forward, backward;

    for(auto it = begin; it != end; ++it)
    {
      forward.push_back(*it);
    }

    for(auto rit = rbegin; rit != rend; ++rit)
    {
      backward.push_back(*rit);
    }

    std::reverse(backward.begin(), backward.end());

    ASSERT_EQ(backward, forward);

    forward.clear();

    begin = ValueIterator(rend.base());
    end = ValueIterator(rbegin.base());

    for(auto it = begin; it != end; ++it)
    {
      forward.push_back(*it);
    }

    ASSERT_EQ(forward, backward);
  }
}
