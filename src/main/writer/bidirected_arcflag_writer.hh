#ifndef BIDIRECTED_ARCFLAG_WRITER_HH
#define BIDIRECTED_ARCFLAG_WRITER_HH

#include <iostream>

#include "graph/graph.hh"
#include "arcflags/arcflags.hh"

class BidirectedArcFlagWriter
{
public:
  void writeBidirectedArcFlags(std::ostream& out,
                               const Bidirected<ArcFlags> arcFlags);

  void writeBidirectedArcFlags(std::ostream& out,
                               const ArcFlags& incomingFlags,
                               const ArcFlags& outgoingFlags);
};


#endif /* BIDIRECTED_ARCFLAG_WRITER_HH */
