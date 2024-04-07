#include "loader.hpp"
#include "mpi.h"
#include "results.hpp"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <fstream>
#include <iostream>

#include "airnow/record.hpp"

std::string recieveString() {
  MPI_Status status;
  MPI_Probe(0, 0, MPI_COMM_WORLD, &status);

  int count;
  MPI_Get_count(&status, MPI_UNSIGNED_CHAR, &count);

  // Allocate buffer for the string + null terminator
  std::vector<unsigned char> buffer(count + 1);
  MPI_Recv(buffer.data(), count, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD,
           MPI_STATUS_IGNORE);
  buffer[count] = '\0'; // Null-terminate the string

  return std::string(buffer.begin(), buffer.end());
}

Stats calculateStats(const std::vector<Record *> &records) {
  Stats stats;

  stats.minAQI = std::numeric_limits<int>::max();
  stats.maxAQI = std::numeric_limits<int>::min();
  stats.aqiSum = 0;
  stats.aqiCount = 0;
  stats.minConcentration = std::numeric_limits<int>::max();
  stats.maxConcentration = std::numeric_limits<int>::min();
  stats.concentrationSum = 0;
  stats.concentrationCount = 0;

  for (auto record : records) {
    if (record->aqi != -999) {
      stats.minAQI = std::min(stats.minAQI, record->aqi);
      stats.maxAQI = std::max(stats.maxAQI, record->aqi);
      stats.aqiSum += record->aqi;
      stats.aqiCount++;
    }

    if (record->concentration != -999) {
      stats.minConcentration =
          std::min(stats.minConcentration, record->concentration);
      stats.maxConcentration =
          std::max(stats.maxConcentration, record->concentration);
      stats.concentrationSum += record->concentration;
      stats.concentrationCount++;
    }
  }
  return stats;
}

Results calculateResults(const std::vector<Record *> &records) {
  std::cout << "Calculating results" << std::endl;
  Results results;
  results.date = records[0]->date;

  results.compiledStatistics = calculateStats(records);
  std::map<std::string, std::vector<Record *>> parameterStatistics;
  for (auto record : records) {
    parameterStatistics[record->parameter].push_back(record);
  }

  for (const auto &[parameter, records] : parameterStatistics) {
    results.parameterStatistics[parameter] = calculateStats(records);
  }

  for (auto record : records) {
    if (record->category > 4) {
      results.stationsWithHealthAlerts.push_back(record->siteName);
    }
  }

  return results;
}

int main(int argc, char *argv[]) {
  // Setup MPI
  int world_rank;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  // Print the world rank

  // Get the start file and end file
  std::string startFile = recieveString();
  std::string endFIle = recieveString();

  std::cout << "C++ Rank: " << world_rank << std::endl
            << "Start file: " << startFile << std::endl
            << "End file: " << endFIle << std::endl;

  Loader loader;
  loader.loadFiles("./data", startFile, endFIle);

  // Send a record to the master node
  for (auto [date, records] : loader.date_records) {
    // Send the results to the master node
    Results results = calculateResults(records);
    std::string resultsJson = results.toJson();
    std::vector<unsigned char> buffer(resultsJson.begin(), resultsJson.end());
    MPI_Send(buffer.data(), buffer.size(), MPI_UNSIGNED_CHAR, 0, 0,
             MPI_COMM_WORLD);
  }

  MPI_Finalize();
  return 0;
}
