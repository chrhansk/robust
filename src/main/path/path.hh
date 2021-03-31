#ifndef PATH_HH
#define PATH_HH

#include <deque>
#include <memory>

#include "graph/edge.hh"
#include "graph/edge_map.hh"

/**
 * A Path given by a tuple of Edge%s. Each successive
 * pair of Edge%s of the Path shares a common Vertex.
 * A Path may contain cylces.
 **/
class Path
{
private:
  std::shared_ptr<std::deque<Edge>> edges;
public:

  /**
   * Constructs an empty Path.
   **/
  Path() : edges(new std::deque<Edge>()) {}

  /**
   * Constructs a path from the given Edges%s.
   **/
  Path(std::initializer_list<Edge> edges);

  /**
   * Appends an Edge to the given Path.
   **/
  void append(const Edge& edge);

  /**
   * Prepends an Edge to the given Path.
   **/
  void prepend(const Edge& edge);

  /**
   * Returns the cost of the path with respect to
   * the given cost function.
   *
   * @tparam A cost function given as a map from Edge%s to numbers.
   *
   **/
  template<class Func>
  num cost(const Func& func) const;

  /**
   * Returns the Edge%s in this Path.
   **/
  const std::deque<Edge>& getEdges() const;

  /**
   * Adds an Edge to this Path from the given Direction. An
   * INCOMING Edge is prepended, an OUTGOING Edge is appended.
   **/
  void add(const Edge& edge, Direction direction);

  /**
   * Returns whether this Path connects the given vertices.
   **/
  bool connects(Vertex source, Vertex target) const;

  /**
   * Returns whether this Path connects the given vertices in the given
   * Direction.
   **/
  bool connects(Vertex source, Vertex target, Direction direction) const;

  /**
   * Returns whether this Path contains the given Vertex.
   **/
  bool contains(Vertex vertex) const;

  /**
   * Returns whether this Path is simple, i.e. whether it does
   * not contains cycles.
   **/
  bool isSimple() const;

  /**
   * Returns the source Vertex of this Path.
   **/
  Vertex getSource() const;

  /**
   * Returns the target Vertex of this Path.
   **/
  Vertex getTarget() const;

  /**
   * Returns the endpoint of this Path with respect to
   * the given Direction-
   **/
  Vertex getEndpoint(Direction direction) const;

  /**
   * Returns whether this Path satisfies the given filter.
   * @tparam Filter A filer given by a map from Edge%s to boolean values.
   **/
  template <class Filter>
  bool satisfies(Filter& filter) const;

  /**
   * Returns whether this Path decomposes into two subpaths such
   * that the subpath containing the source satisfies the given
   * forward filter and the one containing the target satisfies
   * the backward filter.
   *
   * @tparam ForwardFilter  A filer given by a map from Edge%s to boolean values.
   * @tparam BackwardFilter A filer given by a map from Edge%s to boolean values.
   **/
  template <class ForwardFilter, class BackwardFilter>
  bool satisfies(ForwardFilter& forwardFilter,
                 BackwardFilter& backwardFilter) const;

  /**
   * Returns whether this path contains any Edge%s.
   **/
  operator bool() const
  {
    return !(getEdges().empty());
  }
};

template<class Func>
num Path::cost(const Func& func) const
{
  num s = 0;

  for(const Edge& edge : getEdges())
  {
    s += func(edge);
  }

  return s;
}

template <class Filter>
bool Path::satisfies(Filter& filter) const
{
  for(const Edge& edge : getEdges())
  {
    if(!filter(edge))
    {
      return false;
    }
  }

  return true;
}

template <class ForwardFilter, class BackwardFilter>
bool Path::satisfies(ForwardFilter& forwardFilter,
                     BackwardFilter& backwardFilter) const
{
  bool forward = true;

  for(const Edge& edge : getEdges())
  {
    if(forward)
    {
      if(!forwardFilter(edge))
      {
        if(!backwardFilter(edge))
        {
          return false;
        }

        forward = false;
      }
    }
    else if(!backwardFilter(edge))
    {
      return false;
    }
  }

  return true;
}



#endif /* PATH_HH */
