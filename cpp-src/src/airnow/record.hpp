#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <fstream>

#ifndef AIRNOW_RECORD_HPP
#define AIRNOW_RECORD_HPP

class Record {
public:
  double latitude;
  double longitude;
  std::string date;
  std::string parameter;
  int concentration;
  std::string unit;
  int rawConcentration;
  int aqi;
  int category;
  std::string siteName;
  std::string siteAgency;
  std::string siteId;

  Record() = default;

  Record(double latitude, double longitude, const std::string &date,
         const std::string &parameter, int concentration,
         const std::string &unit, int rawConcentration, int aqi, int category,
         const std::string &siteName, const std::string &siteAgency,
         const std::string &siteId)
      : latitude(latitude), longitude(longitude), date(date),
        parameter(parameter), concentration(concentration), unit(unit),
        rawConcentration(rawConcentration), aqi(aqi), category(category),
        siteName(siteName), siteAgency(siteAgency), siteId(siteId) {}

  std::string toJson() const {
    boost::property_tree::ptree pt;

    // Populate the tree
    pt.put("lat", latitude);
    pt.put("lon", longitude);
    pt.put("date", date);
    pt.put("parameter", parameter);
    pt.put("concentration", concentration);
    pt.put("unit", unit);
    pt.put("rawConcentration", rawConcentration);
    pt.put("aqi", aqi);
    pt.put("category", category);
    pt.put("siteName", siteName);
    pt.put("siteAgency", siteAgency);
    pt.put("siteId", siteId);

    // Write to string
    std::ostringstream oss;
    boost::property_tree::write_json(oss, pt, false);
    return oss.str();
  }

  static Record fromJson(const std::string &json) {
    std::istringstream is(json);
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(is, pt);

    Record record;
    record.latitude = pt.get<double>("lat");
    record.longitude = pt.get<double>("lon");
    record.date = pt.get<std::string>("date");
    record.parameter = pt.get<std::string>("parameter");
    record.concentration = pt.get<int>("concentration");
    record.unit = pt.get<std::string>("unit");
    record.rawConcentration = pt.get<int>("rawConcentration");
    record.aqi = pt.get<int>("aqi");
    record.category = pt.get<int>("category");
    record.siteName = pt.get<std::string>("siteName");
    record.siteAgency = pt.get<std::string>("siteAgency");
    record.siteId = pt.get<std::string>("siteId");

    return record;
  }

  static std::string vectorToJson(const std::vector<Record> &records) {
    boost::property_tree::ptree pt;
    boost::property_tree::ptree array_tree;
    for (const auto &record : records) {
      boost::property_tree::ptree record_tree;
      record_tree.put("lat", record.latitude);
      record_tree.put("lon", record.longitude);
      record_tree.put("date", record.date);
      record_tree.put("parameter", record.parameter);
      record_tree.put("concentration", record.concentration);
      record_tree.put("unit", record.unit);
      record_tree.put("rawConcentration", record.rawConcentration);
      record_tree.put("aqi", record.aqi);
      record_tree.put("category", record.category);
      record_tree.put("siteName", record.siteName);
      record_tree.put("siteAgency", record.siteAgency);
      record_tree.put("siteId", record.siteId);
      array_tree.push_back(std::make_pair("", record_tree));
    }
    return "array_tree";
  }

  static std::string vectorToJson(const std::vector<Record *> &records) {
    boost::property_tree::ptree pt;
    boost::property_tree::ptree array_tree;
    for (const auto &record : records) {
      boost::property_tree::ptree record_tree;
      record_tree.put("lat", record->latitude);
      record_tree.put("lon", record->longitude);
      record_tree.put("date", record->date);
      record_tree.put("parameter", record->parameter);
      record_tree.put("concentration", record->concentration);
      record_tree.put("unit", record->unit);
      record_tree.put("rawConcentration", record->rawConcentration);
      record_tree.put("aqi", record->aqi);
      record_tree.put("category", record->category);
      record_tree.put("siteName", record->siteName);
      record_tree.put("siteAgency", record->siteAgency);
      record_tree.put("siteId", record->siteId);
      array_tree.push_back(std::make_pair("", record_tree));
    }

    pt.add_child("records", array_tree);

    // Write to file
    std::ofstream file("output.json");
    boost::property_tree::write_json(file, pt);
    file.close();

    std::ostringstream oss;
    boost::property_tree::write_json(oss, pt, false);
    return oss.str();
  }

  static std::vector<Record> vectorFromJson(const std::string &json) {
    std::istringstream is(json);
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(is, pt);

    std::vector<Record> records;
    for (const auto &record : pt.get_child("records")) {
      records.emplace_back(Record(
          record.second.get<double>("lat"), record.second.get<double>("lon"),
          record.second.get<std::string>("date"),
          record.second.get<std::string>("parameter"),
          record.second.get<int>("concentration"),
          record.second.get<std::string>("unit"),
          record.second.get<int>("rawConcentration"),
          record.second.get<int>("aqi"), record.second.get<int>("category"),
          record.second.get<std::string>("siteName"),
          record.second.get<std::string>("siteAgency"),
          record.second.get<std::string>("siteId")));
    }

    return records;
  }
};

#endif
