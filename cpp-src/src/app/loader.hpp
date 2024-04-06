/**
 * A placeholder for your work
 */

/**
 * A placeholder for your work
 */

#include "omp.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "airnow/FileIterator.hpp"
#include "airnow/record.hpp"

#ifndef LOADER_HPP
#define LOADER_HPP

class Loader {
private:
public:
  // Sorted list of datetimes in the dataset
  std::set<std::string> dates;
  std::set<std::string> siteIds;
  std::set<std::string> parameters;

  // Indexed by date
  std::map<std::string, std::vector<Record *>> date_records;

  // Indexed by date - stationId - parameter
  std::map<std::string, Record> records;
  // std::map<std::string, std::vector<Record>> records;

  int numFiles = 0;

  std::vector<Record *> getRecords(const std::string &date) {
    std::string key = date;
    if (date_records.find(key) != date_records.end()) {
      return date_records[key];
    }
  }

  Record getRecord(const std::string &date, const std::string parameter,
                   const std::string &siteId) {
    std::string key = date + "-" + parameter + "-" + siteId;
    if (records.find(key) != records.end()) {
      return records[key];
    }
  }

  void addRecord(double latitude, double longitude, const std::string &date,
                 const std::string &parameter, int concentration,
                 const std::string &unit, int rawConcentration, int aqi,
                 int category, const std::string &siteName,
                 const std::string &siteAgency, const std::string &siteId) {

    std::string key = date + "-" + siteId + "-" + parameter;

    records[key] =
        Record(latitude, longitude, date, parameter, concentration, unit,
               rawConcentration, aqi, category, siteName, siteAgency, siteId);

    Record *record_ptr = &records[key];

    date_records[date].push_back(record_ptr);

    dates.insert(date);
    dates.insert(parameter);
    dates.insert(siteId);
  }

  void incrementNumFiles() {
#pragma omp atomic
    numFiles++;
  }

  // Assumes that the input string is a valid CSV line
  std::vector<std::string> parseFields(std::string line) {
    std::istringstream iss(line);
    std::string field;
    std::vector<std::string> fields;

    while (std::getline(iss, field, ',')) {
      // If field does not end in a ". i.e. there is a non deliminating comma
      // within a field
      if (!field.empty() && field.front() == '"' && field.back() != '"') {
        // Keep reading until we find a field that ends with a quote
        std::string tempField;
        while (std::getline(iss, tempField, ',')) {
          field += ',' + tempField;
          if (!tempField.empty() && tempField.back() == '"') {
            break;
          }
        }
      }

      // Remove surrounding quotes
      if (!field.empty() && field.front() == '"' && field.back() == '"') {
        field = field.substr(1, field.size() - 2); // Remove surrounding quotes
      }
      fields.push_back(field);
    }

    return fields;
  }

  void loadFile(const std::string &filePath) {
    // Print the file path

    std::ifstream inputFile(filePath);
    if (!inputFile.is_open()) {
      std::cerr << "Error: Unable to open input CSV file." << std::endl;
      return;
    }

    incrementNumFiles();

    // Read the CSV file
    int lineNum = 1;
    std::string line;
    while (std::getline(inputFile, line)) {
      // Parse the line into fields
      std::vector<std::string> fields = parseFields(line);

      // Convert fields to Record and Station objects
      // Check for correct number of fields
      if (fields.size() == 13) {
        double lat = std::stod(fields[0]);
        double lon = std::stod(fields[1]);
        int concentration = std::stoi(fields[4]);
        int rawConcentration = std::stoi(fields[6]);
        int aqi = std::stoi(fields[7]);
        int category = std::stoi(fields[8]);

// Update records map with thread safety
#pragma omp critical(update_records_map)
        {
          // records[fields[2]].push_back(record);
          addRecord(lat, lon, fields[2], fields[3], concentration, fields[5],
                    rawConcentration, aqi, category, fields[9], fields[10],
                    fields[12]);
        }

      } else {
        std::cerr << "Error: Invalid format in CSV file on line# " << lineNum
                  << "." << std::endl;
      }
      lineNum++;
    }
  }

  void loadFiles(const std::string &baseDir, const std::string &startDate,
                 const std::string &endDate) {
    omp_set_num_threads(4);

    // Use file iterator to get a vector of all file paths
    FileIterator fileIterator(baseDir, startDate, endDate);
    std::vector<std::string> filePaths = fileIterator.getAllFilePaths();

// Load files in parallel
#pragma omp parallel for
    for (size_t i = 0; i < filePaths.size(); i++) {
      loadFile(filePaths[i]);
    }
  }
};

#endif

/*

#include "omp.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "airnow/FileIterator.hpp"
#include "airnow/record.hpp"
#include "airnow/station.hpp"

#ifndef LOADER_HPP
#define LOADER_HPP

class Loader {
private:
public:
  // Sorted list of datetimes in the dataset
  std::set<std::string> dates;

  // Indexed by siteId
  std::map<std::string, Station> stations;

  // Indexed by date
  std::map<std::string, std::vector<Record *>> date_records;

  // Indexed by date - stationId
  std::map<std::string, std::vector<Record *>> date_station_records;

  // Indexed by date - stationId - parameter
  std::map<std::string, Record *> records;
  // std::map<std::string, std::vector<Record>> records;

  int numFiles = 0;

  void getRecords(const std::string &date, std::vector<Record *> &records) {
    std::string key = date;
    if (date_records.find(key) != date_records.end()) {
      records = date_records[key];
    }
  }

  void getRecords(const std::string &date, const std::string &siteId,
                  std::vector<Record *> &records) {
    std::string key = date + "-" + siteId;
    if (date_station_records.find(key) != date_station_records.end()) {
      records = date_station_records[key];
    }
  }

  void addRecord(Record record) {
    std::string date = record.date;
    std::string siteId = record.siteId;
    std::string parameter = record.parameter;
    std::string key = date + "-" + siteId + "-" + parameter;

    records[key] = &record;

    key = date + "-" + siteId;
    date_station_records[key].push_back(&record);
    date_records[record.date].push_back(&record);
    dates.insert(record.date);
  }

  void incrementNumFiles() {
#pragma omp atomic
    numFiles++;
  }

  // Assumes that the input string is a valid CSV line
  std::vector<std::string> parseFields(std::string line) {
    std::istringstream iss(line);
    std::string field;
    std::vector<std::string> fields;

    while (std::getline(iss, field, ',')) {
      // If field does not end in a ". i.e. there is a non deliminating comma
      // within a field
      if (!field.empty() && field.front() == '"' && field.back() != '"') {
        // Keep reading until we find a field that ends with a quote
        std::string tempField;
        while (std::getline(iss, tempField, ',')) {
          field += ',' + tempField;
          if (!tempField.empty() && tempField.back() == '"') {
            break;
          }
        }
      }

      // Remove surrounding quotes
      if (!field.empty() && field.front() == '"' && field.back() == '"') {
        field = field.substr(1, field.size() - 2); // Remove surrounding quotes
      }
      fields.push_back(field);
    }

    return fields;
  }

  void loadFile(const std::string &filePath) {
    // Print the file path
    std::cout << "Loading file: " << filePath << std::endl;

    std::ifstream inputFile(filePath);
    if (!inputFile.is_open()) {
      std::cerr << "Error: Unable to open input CSV file." << std::endl;
      return;
    }

    incrementNumFiles();

    // Read the CSV file
    int lineNum = 1;
    std::string line;
    while (std::getline(inputFile, line)) {
      // Parse the line into fields
      std::vector<std::string> fields = parseFields(line);

      // Convert fields to Record and Station objects
      // Check for correct number of fields
      if (fields.size() == 13) {
        double lat = std::stod(fields[0]);
        double lon = std::stod(fields[1]);
        // Create a Record object and add it to the records map
        Record record =
            Record(fields[2], lat, lon, fields[3], fields[5],
                   std::stoi(fields[4]), std::stoi(fields[6]),
                   std::stoi(fields[7]), std::stoi(fields[8]), fields[12]);

// Update records map with thread safety
#pragma omp critical(update_records_map)
        {
          // records[fields[2]].push_back(record);
          addRecord(record);
        }

#pragma omp critical(update_stations_map)
        {
          // If the station does not exist, create a new station object
          if (stations.find(fields[12]) == stations.end()) {
            stations[fields[12]] =
                Station(lat, lon, fields[9], fields[10], fields[12]);
          }
          // Always add the record to the station's records
          stations[fields[12]].records[fields[2]].push_back(&record);
        }

      } else {
        std::cerr << "Error: Invalid format in CSV file on line# " << lineNum
                  << "." << std::endl;
      }
      lineNum++;
    }
  }

  void loadFiles(const std::string &baseDir, const std::string &startDate,
                 const std::string &endDate) {
    omp_set_num_threads(4);

    // Use file iterator to get a vector of all file paths
    FileIterator fileIterator(baseDir, startDate, endDate);
    std::vector<std::string> filePaths = fileIterator.getAllFilePaths();

// Load files in parallel
#pragma omp parallel for
    for (int i = 0; i < filePaths.size(); i++) {
      loadFile(filePaths[i]);
    }
  }
};

#endif

*/