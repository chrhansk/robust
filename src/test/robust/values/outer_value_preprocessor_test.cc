#include "value_preprocessor_test.hh"

#include "robust/values/outer_value_preprocessor.hh"

ADD_VALUE_PREPROCESSOR_TEST(outer_value_preprocessor_test,
                            OuterValuePreprocessor(graph, costs, deviations, deviationSize, partition))

