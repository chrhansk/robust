#ifndef ARCFLAG_PARSER_HH
#define ARCFLAG_PARSER_HH

#include <memory>

#include "graph.pb.h"

#include "graph/graph.hh"
#include "arcflags/partition.hh"
#include "arcflags/arcflags.hh"

class ArcFlagParser
{
public:
  std::unique_ptr<ArcFlags> parseArcFlags(const Graph& graph,
                                          const Partition& partition,
                                          const Protobuf::ArcFlags& PBFArcFlags);
};


#endif /* ARCFLAG_PARSER_HH */
