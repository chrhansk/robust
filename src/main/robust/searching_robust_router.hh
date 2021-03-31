#ifndef SEARCHING_ROBUST_ROUTER_HH
#define SEARCHING_ROBUST_ROUTER_HH

#include "robust_router.hh"
#include "robust_utils.hh"
#include "theta/theta_router.hh"

#include "search_interval.hh"

class SearchingRobustRouter : public RobustRouter
{
public:

  enum Option
  {
    BOUNDING = 1,
    TIGHTENING = 2
  };

  enum IntervalSelection
  {
    LOWEST_BOUND,
    LARGEST_INTERVAL
  };

  typedef FlagSet<Option> Options;

protected:
  struct SearchIntervalCompare
  {
    IntervalSelection selection;

    SearchIntervalCompare(IntervalSelection selection)
      : selection(selection)
    {
    }

    bool operator()(const SearchInterval& first, const SearchInterval& second) const
    {
      return !compare(first, second);
    }

    bool compare(const SearchInterval& first, const SearchInterval& second) const
    {
      switch (selection) {
      case SearchingRobustRouter::LOWEST_BOUND:
        return first.lowerBoundCost < second.lowerBoundCost;
        break;
      case SearchingRobustRouter::LARGEST_INTERVAL:
        return first.getValues().size() > second.getValues().size();
        break;
      }

      throw std::logic_error("");
      return true;
    }
  };

  num getBound(num bound, num bestCost, num value);

  bool useBounds;
  bool tightenIntervals;
  ThetaRouter& router;
  Options options;
  IntervalSelection intervalSelection;

  bool verifyResult(const SearchResult& simpleResult,
                    Vertex source,
                    Vertex target,
                    num value,
                    num costBound);

public:
  SearchingRobustRouter(const Graph& graph,
                        const EdgeFunc<num>& costs,
                        const EdgeFunc<num>& deviations,
                        idx deviationSize,
                        ThetaRouter& router,
                        Options options = Options());

  using RobustRouter::shortestPath;

  RobustSearchResult shortestPath(Vertex source,
                                  Vertex target,
                                  const ValueVector& possibleValues,
                                  num bound) override;

  bool doesUseBounds() const;
  void setUseBounds(bool useBounds);

  bool doesTightenIntervals() const;
  void setTightenIntervals(bool tightenIntervals);
};

#endif /* SEARCHING_ROBUST_ROUTER_HH */
