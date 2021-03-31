#ifndef ROBUST_CONTRACTION_ROUTER_HH
#define ROBUST_CONTRACTION_ROUTER_HH

#include "robust/theta_router.hh"

class RobustContractionPreprocessor;

class RobustContractionRouter : public ThetaRouter
{
private:
  const RobustContractionPreprocessor& preprocessor;

  template<bool bounded,
           bool stalling>
  SearchResult findShortestPath(Vertex source,
                                Vertex target,
                                num thetaValue,
                                num bound = inf);

public:
  RobustContractionRouter(const RobustContractionPreprocessor& preprocessor)
    : preprocessor(preprocessor)
  {}

  SearchResult shortestPath(Vertex source,
                            Vertex target,
                            num theta,
                            num bound) override;

  SearchResult shortestPath(Vertex source,
                            Vertex target,
                            num theta) override;
};


#endif /* ROBUST_CONTRACTION_ROUTER_HH */
