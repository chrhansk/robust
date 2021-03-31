#ifndef EDGE_MAP_HH
#define EDGE_MAP_HH

#include <iostream>
#include <cassert>

#include "graph/graph.hh"

/**
 * A class modelling a function defined on the edge set of a graph.
 */
template <class T>
class EdgeFunc
{
public:
  virtual T operator()(const Edge& edge) const = 0;
  virtual ~EdgeFunc() {}
};


/**
 * A comparator for edges based on a function mapping to comparable objects.
 */
template <class T>
class EdgeCompare
{
private:
  const EdgeFunc<T>& func;

public:
  EdgeCompare(const EdgeFunc<T>& func)
    : func(func) {}

  bool operator()(const Edge& first, const Edge& second) const
  {
    return func(first) < func(second);
  }
};

template <class T>
class EdgeMap;

/**
 * A class which returns copies of the values of an underlying map.
 */
template <class T>
class EdgeValueMap : public EdgeFunc<T>
{
private:
  const EdgeMap<T> *map;

public:
  EdgeValueMap(const EdgeMap<T> & map) : map(&map) {}

  T operator()(const Edge& edge) const override
  {
    return T((*map)(edge));
  }

  ~EdgeValueMap() {}
};


/**
 * A map which explicitely stores its values.
 * The contents of the map can be modified.
 */
template <class T>
class EdgeMap : public EdgeFunc<const T&>
{
private:
  std::vector<T> values;

public:
  EdgeMap(const Graph& graph, T value)
    : values(graph.getEdges().size(), value)
  {
  }

  EdgeMap(const EdgeMap<T>& other) = default;

  EdgeMap(const Graph& graph, const EdgeFunc<T>& other)
  {
    for(const Edge& edge : graph.getEdges())
    {
      values.push_back(other(edge));
    }
  }

  EdgeMap() {}

  T& operator()(const Edge& edge)
  {
    assert(edge.getIndex() >= 0 and
           edge.getIndex() < values.size());
    return values[edge.getIndex()];
  }

  const T& operator()(const Edge& edge) const override
  {
    assert(edge.getIndex() >= 0 and
           edge.getIndex() < values.size());
    return values[edge.getIndex()];
  }

  void setValue(const Edge& edge, const T& value)
  {
    values[edge.getIndex()] = value;
  }

  void reset(const T& value)
  {
    std::fill(values.begin(), values.end(), value);
  }

  EdgeValueMap<T> getValues() const
  {
    return EdgeValueMap<T>(*this);
  }

  void extend(const Edge& edge, T value)
  {
    while(values.size() <= edge.getIndex())
    {
      values.push_back(value);
    }
  }
};

#endif /* EDGE_MAP_HH */
