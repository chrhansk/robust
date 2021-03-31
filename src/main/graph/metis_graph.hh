#ifndef METIS_GRAPH_HH
#define METIS_GRAPH_HH

#include "graph.hh"

namespace METIS
{
  extern "C"
  {
  #include <metis.h>
  }
}

class METISGraph
{
private:
  METIS::idx_t* xadj;
  METIS::idx_t* adjncy;
  METIS::idx_t numVertices, numEdges;

public:
  METISGraph(const Graph& graph);

  ~METISGraph();

  METIS::idx_t* getXAdj() const;
  METIS::idx_t* getAdjncy() const;

  METIS::idx_t getNumEdges() const;
  METIS::idx_t getNumVertices() const;
};


#endif /* METIS_GRAPH_HH */
