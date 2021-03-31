#ifndef ARCFLAG_TEST_HH
#define ARCFLAG_TEST_HH

#include "basic_test.hh"

#include "arcflags/arcflag_preprocessor.hh"
#include "arcflags/geometric_partition.hh"

class ArcFlagTest : public BasicRouterTest
{
protected:
  GeometricPartion partition;
  ArcFlagPreprocessor preprocessor;
public:
  ArcFlagTest()
    : partition(graph, points, 5),
      preprocessor(graph, costs, partition)
  {}

};

#endif /* ARCFLAG_TEST_HH */
