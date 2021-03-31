#ifndef BIDIRECTED_ARCFLAG_COMPOSER_HH
#define BIDIRECTED_ARCFLAG_COMPOSER_HH

#include "graph.pb.h"

#include "arcflags/arcflags.hh"

class BidirectedArcFlagComposer
{
public:
  void composeBidirectedArcFlags(const Bidirected<ArcFlags> arcFlags,
                                 Protobuf::BidirectionalArcFlags& PBFArcFlags);

  void composeBidirectedArcFlags(const ArcFlags& incomingFlags,
                                 const ArcFlags& outgoingFlags,
                                 Protobuf::BidirectionalArcFlags& PBFArcFlags);
};


#endif /* BIDIRECTED_ARCFLAG_COMPOSER_HH */
