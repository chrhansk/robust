#ifndef VERTEX_MAP_HH
#define VERTEX_MAP_HH

#include "graph/graph.hh"

template <class T>
class VertexFunc
{
public:
  virtual T operator()(const Vertex& vertex) const = 0;
  virtual ~VertexFunc() {}
};

template<class T, class Combination>
class CombinedVertexFunc : public VertexFunc<T>
{
private:
  Combination combination;
  const VertexFunc<T>& first;
  const VertexFunc<T>& second;
public:
  CombinedVertexFunc(Combination combination,
                     const VertexFunc<T>& first,
                     const VertexFunc<T>& second)
    : combination(combination),
      first(first),
      second(second)
  {}

  T operator()(const Vertex& vertex) const override
  {
    return combination(first(vertex), second(vertex));
  }
};

template <class T, bool decreasing = false>
class VertexCompare
{
private:
  const VertexFunc<T>& func;

public:
  VertexCompare(const VertexFunc<T>& func)
    : func(func) {}

  bool operator()(const Vertex& first, const Vertex& second) const
  {
    if(decreasing)
    {
      return func(first) > func(second);
    }
    else
    {
      return func(first) < func(second);
    }

  }
};

template <class T>
class VertexMap;

template <class T>
class VertexValueMap : public VertexFunc<T>
{
private:
  const VertexMap<T> *map;

public:
  VertexValueMap(const VertexMap<T> & map) : map(&map) {}

  T operator()(const Vertex& vertex) const override
  {
    return T((*map)(vertex));
  }

  ~VertexValueMap() {}
};

template <class T>
class VertexMap : public VertexFunc<const T&>
{
private:
  std::vector<T> values;

public:
  VertexMap(const Graph& graph, T value)
    : values(graph.getVertices().size(), value)
  {
  }

  VertexMap()
  {}

  T& operator()(const Vertex& vertex)
  {
    return values[vertex.getIndex()];
  }

  const T& operator()(const Vertex& vertex) const override
  {
    return values[vertex.getIndex()];
  }

  void setValue(const Vertex& vertex, const T& value)
  {
    values[vertex.getIndex()] = value;
  }

  void reset(const T& value)
  {
    std::fill(values.begin(), values.end(), value);
  }

  VertexValueMap<T> getValues() const
  {
    return VertexValueMap<T>(*this);
  }
};

#endif /* VERTEX_MAP_HH */
