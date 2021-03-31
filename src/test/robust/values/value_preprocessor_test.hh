#ifndef VALUE_PREPROCESSOR_TEST_HH
#define VALUE_PREPROCESSOR_TEST_HH

#include "basic_test.hh"

#include "arcflags/metis_partition.hh"
#include "robust/robust_utils.hh"
#include "robust/theta/theta_router.hh"

class ValuePreprocessorTest : public BasicTest
{
protected:
  METISPartition partition;
  const Region& sourceRegion;
  const Region& targetRegion;
public:
  ValuePreprocessorTest();

  void testRouter(const ValueVector& requiredValues);

  void testRouter(const Region& sourceRegion,
                  const Region& targetRegion,
                  const ValueVector& requiredValues);

  void testRouter(const Region& sourceRegion,
                  const Region& targetRegion,
                  ThetaRouter& thetaRouter,
                  const ValueVector& requiredValues);
};

#define ADD_VALUE_PREPROCESSOR_TEST(test_name, preprocessor_init) \
                                                                  \
  TEST_F(ValuePreprocessorTest, Test_##test_name)                 \
  {                                                               \
    auto preprocessor = (preprocessor_init);                      \
    ValueSet requiredValuesSet =                                  \
      preprocessor.requiredValues(sourceRegion, targetRegion);    \
                                                                  \
    ValueVector requiredValues(requiredValuesSet.begin(),         \
                               requiredValuesSet.end());          \
                                                                  \
    testRouter(requiredValues);                                   \
  }

#endif /* VALUE_PREPROCESSOR_TEST_HH */
