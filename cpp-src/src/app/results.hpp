
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <fstream>
#include <map>
#include <string>
#include <vector>

struct AggregateStats {
  float mean;
  float stdDev;
  int min;
  int max;
  int count;
  int sum;
};

struct RecordStats {
  AggregateStats aqi;
  AggregateStats concentration;
};

class Results {
public:
  std::string date;
  AggregateStats aqi;
  AggregateStats concentration;
  std::map<std::string, RecordStats> parameter_stats;

  AggregateStats baseline_stats;

  std::string toJson() const {
    boost::property_tree::ptree pt;

    // Populate the tree
    pt.put("date", date);

    // Compiled statistics
    boost::property_tree::ptree aqi_tree;
    aqi_tree.put("mean", aqi.mean);
    aqi_tree.put("stdDev", aqi.stdDev);
    aqi_tree.put("min", aqi.min);
    aqi_tree.put("max", aqi.max);
    aqi_tree.put("count", aqi.count);
    aqi_tree.put("sum", aqi.sum);
    pt.add_child("aqi", aqi_tree);

    boost::property_tree::ptree concentration_tree;
    concentration_tree.put("mean", concentration.mean);
    concentration_tree.put("stdDev", concentration.stdDev);
    concentration_tree.put("min", concentration.min);
    concentration_tree.put("max", concentration.max);
    concentration_tree.put("count", concentration.count);
    concentration_tree.put("sum", concentration.sum);
    pt.add_child("concentration", concentration_tree);

    // Compiled statistics
    boost::property_tree::ptree baseline_tree;
    baseline_tree.put("mean", baseline_stats.mean);
    baseline_tree.put("stdDev", baseline_stats.stdDev);
    baseline_tree.put("min", baseline_stats.min);
    baseline_tree.put("max", baseline_stats.max);
    baseline_tree.put("count", baseline_stats.count);
    baseline_tree.put("sum", baseline_stats.sum);
    pt.add_child("baseline_aqi", baseline_tree);

    // Parameter statistics
    boost::property_tree::ptree parameter_stats_tree;
    for (const auto &[parameter, stats] : parameter_stats) {
      boost::property_tree::ptree parameter_tree;

      // AQI
      boost::property_tree::ptree aqi_tree;
      aqi_tree.put("mean", stats.aqi.mean);
      aqi_tree.put("stdDev", stats.aqi.stdDev);
      aqi_tree.put("min", stats.aqi.min);
      aqi_tree.put("max", stats.aqi.max);
      aqi_tree.put("count", stats.aqi.count);
      aqi_tree.put("sum", stats.aqi.sum);
      parameter_tree.add_child("aqi", aqi_tree);

      // Concentration
      boost::property_tree::ptree concentration_tree;
      concentration_tree.put("mean", stats.concentration.mean);
      concentration_tree.put("stdDev", stats.concentration.stdDev);
      concentration_tree.put("min", stats.concentration.min);
      concentration_tree.put("max", stats.concentration.max);
      concentration_tree.put("count", stats.concentration.count);
      concentration_tree.put("sum", stats.concentration.sum);
      parameter_tree.add_child("concentration", concentration_tree);

      parameter_stats_tree.add_child(parameter, parameter_tree);
    }

    // Write to string
    std::ostringstream oss;
    boost::property_tree::write_json(oss, pt, false);
    return oss.str();
  }
};
