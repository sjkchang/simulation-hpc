
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <fstream>
#include <map>
#include <string>
#include <vector>

struct Stats {
  int minAQI;
  int maxAQI;
  int aqiSum;
  int aqiCount;

  int minConcentration;
  int maxConcentration;
  int concentrationSum;
  int concentrationCount;
};

class Results {
public:
  std::string date;

  // Statistics for all parameters
  Stats compiledStatistics;
  std::map<std::string, Stats> parameterStatistics;

  std::vector<std::string> stationsWithHealthAlerts;

  std::string toJson() const {
    boost::property_tree::ptree pt;

    // Populate the tree
    pt.put("date", date);

    // Compiled statistics
    boost::property_tree::ptree compiledStats;
    compiledStats.put("minAQI", compiledStatistics.minAQI);
    compiledStats.put("maxAQI", compiledStatistics.maxAQI);
    compiledStats.put("aqiSum", compiledStatistics.aqiSum);
    compiledStats.put("aqiCount", compiledStatistics.aqiCount);
    compiledStats.put("minConcentration", compiledStatistics.minConcentration);
    compiledStats.put("maxConcentration", compiledStatistics.maxConcentration);
    compiledStats.put("concentrationSum", compiledStatistics.concentrationSum);
    compiledStats.put("concentrationCount",
                      compiledStatistics.concentrationCount);
    pt.add_child("compiledStatistics", compiledStats);

    // Parameter statistics
    boost::property_tree::ptree parameterStats;
    for (const auto &[parameter, stats] : parameterStatistics) {
      boost::property_tree::ptree pstats;
      pstats.put("minAQI", stats.minAQI);
      pstats.put("maxAQI", stats.maxAQI);
      pstats.put("aqiSum", stats.aqiSum);
      pstats.put("aqiCount", stats.aqiCount);
      pstats.put("minConcentration", stats.minConcentration);
      pstats.put("maxConcentration", stats.maxConcentration);
      pstats.put("concentrationSum", stats.concentrationSum);
      pstats.put("concentrationCount", stats.concentrationCount);

      parameterStats.add_child(parameter, pstats);
    }
    pt.add_child("parameters", parameterStats);

    // Stations with health alerts
    boost::property_tree::ptree healthAlerts;
    for (const auto &station : stationsWithHealthAlerts) {
      healthAlerts.push_back(
          std::make_pair("", boost::property_tree::ptree(station)));
    }
    pt.add_child("stationsWithHealthAlerts", healthAlerts);

    // Write to string
    std::ostringstream oss;
    boost::property_tree::write_json(oss, pt, false);
    return oss.str();
  }
};
