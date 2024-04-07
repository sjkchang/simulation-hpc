#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#ifndef AIRNOW_FILE_ITERATOR_HPP
#define AIRNOW_FILE_ITERATOR_HPP

namespace fs = std::filesystem;

class FileIterator {

private:
  std::string baseDir;
  std::string currentDateTime;
  std::string endDateTime;
  std::vector<int> daysInMonth;

  void incrementHour() {
    // Print the current date-time
    std::cout << "Current date-time: " << currentDateTime << std::endl;

    // YYYYMMDD-HH
    int year = stoi(currentDateTime.substr(0, 4));
    int month = stoi(currentDateTime.substr(4, 2));
    int day = stoi(currentDateTime.substr(6, 2));
    // Skip the "-" character
    int hour = stoi(currentDateTime.substr(9, 2));

    hour += 2;
    if (hour > 23) {
      hour = 1;
      day++;
      if (day > daysInMonth[month - 1]) {
        day = 1;
        month++;
        if (month > 12) {
          month = 1;
          year++;
        }
      }
    }

    char buffer[12]; // Increased size to accommodate the additional characters
    snprintf(buffer, sizeof(buffer), "%04d%02d%02d-%02d", year, month, day,
             hour);
    currentDateTime = std::string(buffer);
  }

public:
  FileIterator(const std::string &baseDir, const std::string &startDate,
               const std::string &endDate)
      : baseDir(baseDir), currentDateTime(startDate), endDateTime(endDate) {

    int year = stoi(startDate.substr(0, 4));
    daysInMonth = {
        31, (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 29 : 28,
        31, 30,
        31, 30,
        31, 31,
        30, 31,
        30, 31};
  }

  std::vector<std::string> getAllFilePaths() {
    std::vector<std::string> filePaths;
    while (hasNext()) {
      std::string filePath = next();
      if (!filePath.empty()) {
        filePaths.push_back(filePath);
      }
    }
    return filePaths;
  }

  std::vector<std::string> getAllFileNames() {
    std::vector<std::string> fileNames;
    while (hasNext()) {
      std::string filePath = next();
      if (!filePath.empty()) {
        std::string fileName = filePath.substr(filePath.find_last_of("/") + 1);
        fileNames.push_back(fileName);
      }
    }
    return fileNames;
  }

  bool hasNext() const { return currentDateTime <= endDateTime; }

  std::string next() {
    if (!hasNext()) {
      return ""; // No more files to process.
    }

    // Remove the hour part of the date-time
    std::string currentDate = currentDateTime.substr(0, 8);
    std::string filePath =
        baseDir + "/" + currentDate + "/" + currentDateTime + ".csv";

    // Prepare for the next call
    incrementHour();

    if (fs::exists(filePath)) {
      return filePath;
    } else {
      return next(); // Skip to the next valid file if the current one doesn't
                     // exist.
    }
  }
};

#endif // AIRNOW_FILE_ITERATOR_HPP