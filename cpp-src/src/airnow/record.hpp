#include <map>
#include <sstream>
#include <string>
#include <vector>

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

  std::string toString() const {
    std::ostringstream oss;
    oss << "Date: " << date << std::endl;
    oss << "Parameter: " << parameter << std::endl;
    oss << "Unit: " << unit << std::endl;
    oss << "Concentration: " << concentration << std::endl;
    oss << "Raw Concentration: " << rawConcentration << std::endl;
    oss << "AQI: " << aqi << std::endl;
    oss << "Category: " << category << std::endl;
    oss << "Site ID: " << siteId << std::endl;
    return oss.str();
  }
};

#endif

/*
#include <map>
#include <sstream>
#include <string>
#include <vector>

#ifndef AIRNOW_RECORD_HPP
#define AIRNOW_RECORD_HPP

class Record {
public:
  std::string date;
  std::string parameter;
  std::string unit;
  int concentration;
  int rawConcentration;
  int AQI;
  int category;
  std::string siteId;

  Record(std::string date, std::string parameter, std::string unit,
         int concentration, int rawConcentration, int AQI, int category,
         std::string siteId)
      : date(date), parameter(parameter), unit(unit),
        concentration(concentration), rawConcentration(rawConcentration),
        AQI(AQI), category(category), siteId(siteId){};

  std::string toString() const {
    std::ostringstream oss;
    oss << "Date: " << date << std::endl;
    oss << "Parameter: " << parameter << std::endl;
    oss << "Unit: " << unit << std::endl;
    oss << "Concentration: " << concentration << std::endl;
    oss << "Raw Concentration: " << rawConcentration << std::endl;
    oss << "AQI: " << AQI << std::endl;
    oss << "Category: " << category << std::endl;
    oss << "Site ID: " << siteId << std::endl;
    return oss.str();
  }
};

#endif

*/