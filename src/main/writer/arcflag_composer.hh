#ifndef ARCFLAG_COMPOSER_HH
#define ARCFLAG_COMPOSER_HH

#include "graph.pb.h"

#include "arcflags/arcflags.hh"

class ArcFlagComposer
{
public:
  void composeArcFlags(const ArcFlags& arcFlags,
                       Protobuf::ArcFlags& PBFArcFlags);
};


#endif /* ARCFLAG_COMPOSER_HH */
