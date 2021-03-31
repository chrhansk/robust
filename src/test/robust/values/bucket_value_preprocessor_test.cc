#include "value_preprocessor_test.hh"

#include "robust/values/bucket_value_preprocessor.hh"

ADD_VALUE_PREPROCESSOR_TEST(bucket_value_preprocessor_test,
                            BucketValuePreprocessor(graph, costs, deviations, deviationSize, partition))

