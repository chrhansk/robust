#ifndef METIS_PARTITION_HH
#define METIS_PARTITION_HH

#include "graph/graph.hh"
#include "graph/vertex_map.hh"
#include "util.hh"

#include "partition.hh"

/**
 * A Partition which is determined by METIS.
 **/
class METISPartition : public Partition
{
public:
  METISPartition(const Graph& graph, int size);
};

#endif /* METIS_PARTITION_HH */
