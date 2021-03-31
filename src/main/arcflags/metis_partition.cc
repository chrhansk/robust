#include "metis_partition.hh"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "log.hh"

#include "graph/metis_graph.hh"

METISPartition::METISPartition(const Graph& graph, int size)
  : Partition(graph)
{
  METIS::idx_t* part;
  METISGraph simpleGraph(graph);

  METIS::idx_t n = simpleGraph.getNumVertices();
  METIS::idx_t c = 1;
  METIS::idx_t part_size = size;
  METIS::idx_t objval = 0;

  part = (METIS::idx_t*) malloc(sizeof(METIS::idx_t) * n);

  METIS::idx_t options[METIS_NOPTIONS];
  METIS::METIS_SetDefaultOptions(options);

  options[METIS::METIS_OPTION_SEED] = 17;
  options[METIS::METIS_OPTION_OBJTYPE] = METIS::METIS_OBJTYPE_CUT;
  //options[METIS::METIS_OPTION_DBGLVL] = METIS::METIS_DBG_INFO;
  options[METIS::METIS_OPTION_NUMBERING] = 0;

  int value = METIS::METIS_PartGraphRecursive(&n, &c,
                                              simpleGraph.getXAdj(),
                                              simpleGraph.getAdjncy(),
                                              NULL, /* vertex weights */
                                              NULL, /* size of vertex weights */
                                              NULL, /* edge weights */
                                              &part_size,
                                              NULL, /* target partition weights */
                                              NULL, /* imbalance upper bounds */
                                              options,
                                              &objval,
                                              part);

  if(value != METIS::METIS_OK)
  {
    throw std::runtime_error("Could not start METIS");
  }

  Log(info) << "Found a partition into "
            << size
            << " regions with an objective value of "
            << objval;

  for(int i = 0; i < size; ++i)
  {
    std::vector<Vertex> vertices;

    for(unsigned int j = 0; j < graph.getVertices().size(); ++j)
    {
      if(part[j] == i)
      {
        vertices.push_back(graph.getVertices()[(idx)j]);
      }
    }

    if(vertices.empty())
    {
      continue;
    }

    addRegion(vertices);
  }

  free(part);
}
