#ifndef REGION_PAIR_MAP_H
#define REGION_PAIR_MAP_H

#include <unordered_map>

#include "util.hh"

#include "region.hh"

template<class R>
class RegionPairMap
{
private:
  typedef std::pair<idx, idx> Pair;
  std::unordered_map<Pair, R, PairHash> map;

public:
  RegionPairMap()
  {}

  RegionPairMap(const RegionPairMap<R>& other) = delete;
  RegionPairMap& operator=(const RegionPairMap<R>& other) = delete;

  RegionPairMap& operator=(RegionPairMap<R>&& other)
  {
    map = std::move(other.map);
    return *this;
  }

  RegionPairMap(RegionPairMap<R>&& other)
    : map(std::move(other.map))
  {}

  const R& operator()(const Region& first, const Region&  second) const
  {
    return map.at(std::make_pair(first.getIndex(), second.getIndex()));
  }

  R& operator()(const Region& first, const Region&  second)
  {
    return map.at(std::make_pair(first.getIndex(), second.getIndex()));
  }

  void put(const Region& first, const Region&  second, const R&value)
  {
    map[std::make_pair(first.getIndex(), second.getIndex())] = value;
  }

};


#endif /* REGION_PAIR_MAP_H */
