#ifndef CONTRACTION_PAIR_HH
#define CONTRACTION_PAIR_HH

#include "graph/graph.hh"
#include "path/path.hh"

/**
 * A ContractionPair is a pair \f$ (u,w) \f$ of vertices
 * such that \f$(u, v)\f$ and \f$(v, w)\f$ are edges
 * of an underlying Graph and there is no witness
 * path for the triple \f$ (u, v, w) \f$.
 **/
class ContractionPair
{
private:
  Vertex source;
  Vertex target;
  Path defaultPath;

public:
  /**
   * Constructs a new ContracionPair.
   **/
  ContractionPair(Vertex source,
                  Vertex target,
                  const Path& defaultPath)
    : source(source),
      target(target),
      defaultPath(defaultPath)
  {}

  /**
   * Returns the Vertex \f$ u \f$.
   **/
  Vertex getSource() const
  {
    return source;
  }

  /**
   * Returns the Vertex \f$ w \f$.
   **/
  Vertex getTarget() const
  {
    return target;
  }

  /**
   * Returns the Path consisting of the edges \f$ (u, v) \f$
   * and \f$ (v, w) \f$
   **/
  const Path& getDefaultPath() const
  {
    return defaultPath;
  }

};

#endif /* CONTRACTION_PAIR_HH */
