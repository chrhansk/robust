#ifndef CONTRACTION_RANGE_HH
#define CONTRACTION_RANGE_HH

#include "robust/robust_utils.hh"

class RobustContractionPreprocessor;

/**
 * The RobustContractionRange is the basic ingredient
 * for the RobustContractionHierarchy. It maintains
 * the data structures required to compute the
 * cost function on a contracted Edge.
 *
 * A ContractionRange consists of an interval of
 * values for which the associated Edge is needed
 * in the RobustContractionHierarchy. Additionally,
 * the ContractionRange stores the deviation values
 * of all original Edge%s corresponding to the
 * associated contracted Edge. This is necessary in
 * order to be able to compute reduced costs.
 **/

class ContractionRange
{
private:
  ValueIterator begin, end;
  ValueVector values;
  idx slope;
  num cost;
public:
  ContractionRange()
    : slope(0),
      cost(0)
  {}

  ContractionRange(ValueIterator begin,
                   ValueIterator end,
                   num cost)
    : begin(begin),
      end(end),
      slope(0),
      cost(cost)
  {
    assert(begin <= end);
    assert(getMaximum() >= getMinimum());
  }

  /**
   * Returns an iterator to the begin of the
   * interval of this ContractionRange.
   **/
  const ValueIterator& getBegin() const
  {
    return begin;
  }

  /**
   * Returns the base cost of the associated
   * Edge.
   **/
  num getCost() const
  {
    return cost;
  }

  num& getCost()
  {
    return cost;
  }

  /**
   * Returns the reduced cost of the associated
   * Edge. @see ReducedCosts
   **/
  num getReducedCost(num thetaValue) const
  {
    num sum = getCost();

    assert(thetaValue >= getMinimum() and
           thetaValue <= getMaximum());

    for(const num& value : getValues())
    {
      if(value <= thetaValue)
      {
        break;
      }

      sum += (value - thetaValue);
    }

    sum += -(getSlope() * thetaValue);

    return sum;
  }

  /**
   * Returns the slope of this ContractionRange.
   * The ContractionRange only stores the values
   * in the corresponding interval which fall
   * into the comprising interval. The slope
   * is needed in order to reconstruct the
   * reduced costs.
   **/
  const idx& getSlope() const
  {
    return slope;
  }

  idx& getSlope()
  {
    return slope;
  }

  /**
   * Returns the deviation values of the Edge%s
   * which correspond to the associated Edge.
   **/
  const ValueVector& getValues() const
  {
    return values;
  }

  ValueVector& getValues()
  {
    return values;
  }

  /**
   * Returns the minimal value for which
   * the associated Edge is required in the
   * RobustContractionHierarchy.
   **/
  num getMinimum() const
  {
    ReverseValueIterator revBegin(getEnd());
    return *revBegin;
  }

  /**
   * Returns the maximal value for which
   * the associated Edge is required in the
   * RobustContractionHierarchy.
   **/
  num getMaximum() const
  {
    return *begin;
  }

  /**
   * Returns an iterator past the end of the
   * interval of this ContractionRange.
   **/
  const ValueIterator& getEnd() const
  {
    return end;
  }

  void setBegin(const ValueIterator& it)
  {
    assert(begin <= it);
    end = it;
  }

  void setEnd(const ValueIterator& it)
  {
    assert(begin <= it);
    end = it;
  }

  /**
   * Returns whether the given thetaValue
   * is contained in this ContractionRange.
   * The thetaValue is contained iff the
   * associated Edge is required for the
   * hierarchy corresponding to the
   * given thetaValue.
   **/
  bool contains(num thetaValue) const
  {
    return thetaValue >= getMinimum() and
      thetaValue <= getMaximum();
  }

  /**
   * Returns the size of the interval for which
   * the associated Edge is required.
   **/
  idx rangeSize() const
  {
    return std::distance(getBegin(), getEnd());
  }
};


/**
 * A class which extends the ReducedCosts of the
 * underlying original graph to the
 * RobustContractionHierarchy.
 **/
class ReducedContractionCosts : public EdgeFunc<num>
{
private:
  const EdgeFunc<const ContractionRange&>& contractionRanges;
  num thetaValue;
public:
  ReducedContractionCosts(const EdgeFunc<const ContractionRange&>& contractionRanges,
                          num thetaValue)
    : contractionRanges(contractionRanges),
      thetaValue(thetaValue)
  {}
  num operator()(const Edge& edge) const override
  {
    return contractionRanges(edge).getReducedCost(thetaValue);
  }
};

/**
 * A class which extends the bas costs of the
 * underlying original graph to the
 * RobustContractionHierarchy.
 **/
class OverlayCosts : public EdgeFunc<num>
{
private:
  const EdgeFunc<const ContractionRange&>& contractionRanges;
public:
  OverlayCosts(const EdgeFunc<const ContractionRange&>& contractionRanges)
    : contractionRanges(contractionRanges)
  {}

  num operator()(const Edge& edge) const override
  {
    return contractionRanges(edge).getCost();
  }
};

class EdgeValueFiter
{
private:
  const num upperBound;
  const num thetaValue;
  const EdgeFunc<num>& deviations;
  const EdgeFunc<const ContractionRange&>& contractionRanges;
public:
  EdgeValueFiter(const num upperBound,
                 const num thetaValue,
                 const EdgeFunc<num>& deviations,
                 const EdgeFunc<const ContractionRange&>& contractionRanges)
    : upperBound(upperBound),
      thetaValue(thetaValue),
      deviations(deviations),
      contractionRanges(contractionRanges)
  {}

  bool operator()(const Edge& edge)
  {
    const ContractionRange& range = contractionRanges(edge);

    return deviations(edge) < upperBound and
                              thetaValue <= range.getMaximum() and
      thetaValue >= range.getMinimum();
  }
};

#endif /* CONTRACTION_RANGE_HH */
