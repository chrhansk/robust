#ifndef BIDIRECTED_ARCFLAG_READER_HH
#define BIDIRECTED_ARCFLAG_READER_HH

#include <iostream>
#include <memory>

#include "graph/graph.hh"
#include "arcflags/arcflags.hh"

class BidirectedArcFlagResult
{
public:
  BidirectedArcFlagResult(std::unique_ptr<Partition> partition,
                          std::unique_ptr<ArcFlags> incomingFlags,
                          std::unique_ptr<ArcFlags> outgoingFlags)
    : partition(std::move(partition)),
      incomingFlags(std::move(incomingFlags)),
      outgoingFlags(std::move(outgoingFlags))
  {}

  std::unique_ptr<Partition> partition;
  std::unique_ptr<ArcFlags> incomingFlags;
  std::unique_ptr<ArcFlags> outgoingFlags;
};

class BidirectedArcFlagReader
{
public:
  BidirectedArcFlagResult readBidirectedArcFlags(const Graph& graph, std::istream& in);
};



#endif /* BIDIRECTED_ARCFLAG_READER_HH */
