#ifndef PARTITION_COMPOSER_HH
#define PARTITION_COMPOSER_HH

#include "graph.pb.h"

#include "arcflags/partition.hh"

class PartitionComposer
{
public:
  void composePartition(const Partition& partition,
                        Protobuf::Partition& PBFPartition);
};


#endif /* PARTITION_COMPOSER_HH */
