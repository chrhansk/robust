#ifndef REGION_MAP_H
#define REGION_MAP_H

#include "region.hh"
#include "partition.hh"

template <class T>
class RegionFunc
{
public:
  virtual T operator()(const Region& region) const = 0;
  virtual ~RegionFunc() {}
};

template <class T>
class RegionMap;

template <class T>
class RegionValueMap : public RegionFunc<T>
{
private:
  const RegionMap<T> *map;

public:
  RegionValueMap(const RegionMap<T> & map) : map(&map) {}

  T operator()(const Region& region) const override
  {
    return T((*map)(region));
  }

  ~RegionValueMap() {}
};

template <class T>
class RegionMap : public RegionFunc<const T&>
{
private:
  std::vector<T> values;

public:
  RegionMap(const Partition& partition, T value)
    : values(partition.getRegions().size(), value)
  {
  }

  RegionMap()
  {}

  T& operator()(const Region& region)
  {
    return values[region.getIndex()];
  }

  const T& operator()(const Region& region) const override
  {
    return values[region.getIndex()];
  }

  void setValue(const Region& region, const T& value)
  {
    values[region.getIndex()] = value;
  }

  void reset(const T& value)
  {
    std::fill(values.begin(), values.end(), value);
  }

  RegionValueMap<T> getValues() const
  {
    return RegionValueMap<T>(*this);
  }
};


#endif /* REGION_MAP_H */
