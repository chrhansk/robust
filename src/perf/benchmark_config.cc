#include "benchmark_config.hh"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <string>
#include <set>
#include <exception>
#include <iostream>
namespace pt = boost::property_tree;

BenchmarkSettings readSettings(pt::ptree& tree)
{
  return BenchmarkSettings{
      tree.get("minIterations", 10),
      tree.get("minSeconds", 10.),
      tree.get("numBuckets", 10),
      tree.get("sampleSize", 10),
      tree.get("deviationSize", 5)
  };
}

BenchmarkConfig BenchmarkConfig::readConfig(const std::string& filename)
{
  pt::ptree tree;

  pt::read_json(filename, tree);

  BenchmarkConfig config;

  config.instance = tree.get<std::string>("instance");

  config.settings = readSettings(tree.get_child("settings"));

  return config;
}
