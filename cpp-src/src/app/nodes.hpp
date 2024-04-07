#include "airnow/record.hpp"
#include "loader.hpp"
#include "omp.h"
#include "results.hpp"
#include <iostream>
#include <string>
#include <vector>

class Worker {
public:
  int rank;
  int size;
  std::string base_input_dir;

  std::vector<Record> records;

  Worker(int rank, int size) {
    this->rank = rank;
    this->size = size;
  }

  void run() {
    while (true) {
      // Receive the file name for the worker to process
      std::string fileToProcess = recieveString();

      /// Remove the null terminator
      if (!fileToProcess.empty() && fileToProcess.back() == '\0') {
        fileToProcess.pop_back();
      }

      // Terminating condition
      if (fileToProcess == "End of Work") {
        std::cout << "Worker " << rank << " is terminating" << std::endl;
        break;
      }

      // Load the file
      std::vector<Record> records = Loader::loadFile(fileToProcess);

      Results results;
      results.date = records[0].date;

      // Calculate the average AQI
      AggregateStats aqi = calculateAQIStats(records);
      AggregateStats con = calculateConcentrationStats(records);
      std::map<std::string, RecordStats> parameter_stats =
          calculateParameterStats(records);

      results.aqi = aqi;
      results.concentration = con;
      results.parameter_stats = parameter_stats;

      // Send the results back to the master
      std::string resultsJson = results.toJson();
      std::vector<unsigned char> buffer(resultsJson.begin(), resultsJson.end());
      MPI_Send(buffer.data(), buffer.size(), MPI_UNSIGNED_CHAR, 0, 0,
               MPI_COMM_WORLD);
    }
  }

  // Receive the file name for the worker to process
  void receiveData() {
    std::cout << "Worker " << rank << " is receiving data" << std::endl;
  }

  AggregateStats calculateAQIStats(std::vector<Record> records) {
    omp_set_num_threads(4);

    int count = 0;
    int sum = 0;
    int max = INT_MIN;
    int min = INT_MAX;

#pragma omp parallel for reduction(+ : count, sum) reduction(max:max) reduction(min:min)
    for (int i = 0; i < records.size(); i++) {
      if (records[i].aqi != -999) {
        count++;
        sum += records[i].aqi;
        max = std::max(max, records[i].aqi);
        min = std::min(min, records[i].aqi);
      }
    }

    AggregateStats stats;

    if (count == 0) {
      stats.mean = 0;
      stats.count = 0;
      stats.sum = 0;
      stats.max = 0;
      stats.min = 0;
      return stats;
    }
    stats.mean = sum / count;
    stats.count = count;
    stats.sum = sum;
    stats.max = max;
    stats.min = min;

    return stats;
  }

  AggregateStats calculateConcentrationStats(std::vector<Record> records) {
    omp_set_num_threads(4);

    int count = 0;
    int sum = 0;
    int max = INT_MIN;
    int min = INT_MAX;

#pragma omp parallel for reduction(+ : count, sum) reduction(max:max) reduction(min:min)
    for (int i = 0; i < records.size(); i++) {
      if (records[i].concentration != -999) {
        count++;
        sum += records[i].concentration;
        max = std::max(max, records[i].concentration);
        min = std::min(min, records[i].concentration);
      }
    }
    AggregateStats stats;

    if (count == 0) {
      stats.mean = 0;
      stats.count = 0;
      stats.sum = 0;
      stats.max = 0;
      stats.min = 0;
      return stats;
    }

    stats.mean = sum / count;
    stats.count = count;
    stats.sum = sum;
    stats.max = max;
    stats.min = min;

    return stats;
  };

  std::map<std::string, RecordStats>
  calculateParameterStats(std::vector<Record> records) {
    std::map<std::string, std::vector<Record>> parameter_records;
    // Add the records to the map
    for (auto &record : records) {
      parameter_records[record.parameter].push_back(record);
    }

    std::map<std::string, RecordStats> parameter_stats;
    for (auto &pair : parameter_records) {
      parameter_stats[pair.first].aqi = calculateAQIStats(pair.second);
      parameter_stats[pair.first].concentration =
          calculateConcentrationStats(pair.second);
    }

    return parameter_stats;
  }

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
};