#ifndef UTIL_HH
#define UTIL_HH

#include <algorithm>
#include <bitset>
#include <cstdint>
#include <random>
#include <unordered_map>

#include "defs.hh"

/** @file **/

typedef uint32_t idx;
typedef int32_t num;
typedef unsigned int uint;

extern num inf;

/**
 * A class modelling a Point consisting of
 * two coordinates.
 **/
class Point
{
private:
  float x, y;

public:
  Point(float x, float y)
    : x(x), y(y) {}

  float getX() const
  {
    return x;
  }

  float getY() const
  {
    return y;
  }
};

namespace std
{
  template <class T>
  inline void hash_combine(std::size_t & seed, const T & v)
  {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }
}


struct PairHash {
  typedef std::size_t result_type;

  template <class T1, class T2>
  std::size_t operator () (const std::pair<T1, T2> &p) const {
    result_type seed = 0;
    std::hash_combine(seed, std::hash<T1>{}(p.first));
    std::hash_combine(seed, std::hash<T2>{}(p.second));

    return seed;
  }
};

template <class T, class R>
class PairMap
{
public:
  typedef std::pair<T, T> Pair;
private:
  std::unordered_map<Pair, R, PairHash> map;

public:
  const R& operator()(const T& first, const T& second) const
  {
    return map.at(std::make_pair(first, second));
  }

  R& operator()(const T& first, const T& second)
  {
    return map.at(std::make_pair(first, second));
  }

  void put(const T&first, const T& second, const R&value)
  {
    map[std::make_pair(first, second)] = value;
  }
};

/**
 * Shuffles a range based on a given seed.
 **/
template <class RAIter>
void shuffle(RAIter begin, RAIter end, int seed = 42)
{
  auto engine = std::default_random_engine(seed);

  std::shuffle(begin, end, engine);
}

/**
 * Returns whether the debug mode is enabled.
 **/
constexpr bool debuggingEnabled()
{
#if defined(NDEBUG)
  return false;
#else
  return true;
#endif
}

template <typename TENUM>
class FlagSet {

private:
  using TUNDER = typename std::underlying_type<TENUM>::type;
  std::bitset<std::numeric_limits<TUNDER>::digits> flags;

public:
  FlagSet() = default;

  FlagSet(const FlagSet& other) = default;

  template <typename... ARGS>
  FlagSet(TENUM f, ARGS... args) : FlagSet(args...)
  {
    set(f);
  }

  FlagSet& set(TENUM f)
  {
    flags.set(static_cast<TUNDER>(f));
    return *this;
  }

  FlagSet& clear(TENUM f)
  {
    flags.reset(static_cast<TUNDER>(f));
    return *this;
  }

  FlagSet& operator|=(TENUM f)
  {
    return set(f);
  }

  FlagSet operator|(TENUM f)
  {
    FlagSet newSet(*this);
    newSet |= f;
    return newSet;
  }

  FlagSet& operator&=(TENUM f)
  {
    return clear(f);
  }

  FlagSet operator&(TENUM f)
  {
    FlagSet newSet(*this);
    newSet &= f;
    return newSet;
  }

  operator bool() const
  {
    return flags.any();
  }
};

#endif /* UTIL_HH */
