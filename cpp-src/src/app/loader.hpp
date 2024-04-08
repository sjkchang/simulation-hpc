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
  // Assumes that the input string is a valid CSV line
  static std::vector<std::string> parseFields(std::string line) {
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

  static std::vector<Record> loadFile(const std::string &filePath) {
    // Print the file path

    std::ifstream inputFile(filePath);
    if (!inputFile.is_open()) {
      std::cerr << "Error: Unable to open input CSV file." << std::endl;
      throw std::runtime_error("Error: Unable to open input CSV file.");
    }

    std::vector<Record> records;

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

        records.emplace_back(lat, lon, fields[2], fields[3], concentration,
                             fields[5], rawConcentration, aqi, category,
                             fields[9], fields[10], fields[12]);
      } else {
        std::cerr << "Error: Invalid format in CSV file on line# " << lineNum
                  << "." << std::endl;
      }
      lineNum++;
    }
    return records;
  }

  static std::map<std::string, std::vector<Record>>
  loadFiles(const std::string &baseDir, const std::string &startDateTime,
            const std::string &endDateTime) {
    FileIterator fileIterator(baseDir, startDateTime, endDateTime);
    std::vector<std::string> filePaths = fileIterator.getAllFilePaths();

    std::map<std::string, std::vector<Record>> dateRecords;
    for (const auto &filePath : filePaths) {
      std::vector<Record> records = loadFile(filePath);
      if (!records.empty()) {
        dateRecords[records[0].date] = records;
      }
    }
    return dateRecords;
  }
};

#endif