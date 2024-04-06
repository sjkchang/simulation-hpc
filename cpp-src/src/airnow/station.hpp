#include "FileIterator.hpp"
#include "omp.h"
#include "record.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#ifndef STATION_HPP
#define STATION_HPP

class Station {
public:
  double latitude;
  double longitude;
  std::string site_name;
  std::string site_agency;
  std::string AQS_ID;

  // Indexed by date, One station can have multiple records for a given datetime
  // if it measures multiple pollutants
  std::map<std::string, std::vector<Record *>> records;

  Station() = default;
  Station(double lat, double lon, const std::string &site_name,
          const std::string &site_agency, const std::string &AQS_ID)
      : latitude(lat), longitude(lon), site_name(site_name),
        site_agency(site_agency), AQS_ID(AQS_ID) {}

  std::string toString() const {
    std::ostringstream oss;
    oss << "{ Station: "
        << "latitude: " << latitude << ", "
        << "longitude: " << longitude << ", "
        << "site name: " << site_name << ", "
        << "AQS ID: " << AQS_ID << ", "
        << " }" << std::endl;
    return oss.str();
  }
};

#endif