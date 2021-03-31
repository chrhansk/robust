#ifndef PARTITION_PARSER_HH
#define PARTITION_PARSER_HH

#include <memory>

#include "graph.pb.h"

#include "graph/graph.hh"
#include "arcflags/partition.hh"

class PartitionParser
{
public:
  std::unique_ptr<Partition> parsePartition(const Graph& graph,
                                            const Protobuf::Partition& PBFPartition);
};


#endif /* PARTITION_PARSER_HH */
