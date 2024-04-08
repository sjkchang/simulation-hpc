#include "airnow/record.hpp"
#include "loader.hpp"
#include "omp.h"
#include "results.hpp"
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/named_semaphore.hpp>
#include <iostream>
#include <string>
#include <vector>

using namespace boost::interprocess;

class Worker {
public:
  int rank;
  int size;
  std::string base_input_dir;

  std::vector<Record> records;
  std::map<std::string, std::vector<Record>> baselineRecords;
  // Name of the shared memory segment
  const char *sharedMemoryName = "BaselineData";

  Worker(int rank, int size) {
    this->rank = rank;
    this->size = size;
  }

  // Load the base line data
  // Store in shared memory
  // Check if shared memory is allocated on this node, If not, laad the data
  void loadBaseLine() {

    // Create a named semaphore to synchronize access to shared memory
    const char *semaphoreName = "BaselineSemaphore";
    named_semaphore semaphore(open_or_create, semaphoreName, 1);
    semaphore.wait();

    // Try to create a shared memory segment
    // If it already exists, open it
    try {
      shared_memory_object shm(create_only, sharedMemoryName, read_write);

      // Load the baseline data
      baselineRecords =
          Loader::loadFiles("./data", "20200810-01", "20200810-23");

      // Convert the records to a JSON string and write to shared memory
      std::string json_records = Record::mapToJson(baselineRecords);
      shm.truncate(json_records.size() + 1);
      mapped_region region(shm, read_write);
      std::memcpy(region.get_address(), json_records.c_str(),
                  json_records.size() + 1);

      std::cout << "Shared memory segment created, string written."
                << std::endl;

      // Release the semaphore to allow other processes to access the shared
      // memory
      semaphore.post();
    } catch (interprocess_exception &ex) {

      try {
        // Open the existing shared memory object
        shared_memory_object shm(open_only, sharedMemoryName, read_write);
        mapped_region region(shm, read_write);

        // Read the JSON string from shared memory and convert to records
        char *buffer = static_cast<char *>(region.get_address());
        std::string json_records(buffer);
        baselineRecords = Record::mapFromJson(json_records);

        std::cout << "Shared memory segment opened, string read."
                  << "Number of records: " << baselineRecords.size()
                  << std::endl;

        // Release the semaphore to allow other processes to access the shared
        // memory
        semaphore.post();

      } catch (interprocess_exception &innerEx) {
        std::cerr << "Failed to open existing shared memory segment: "
                  << innerEx.what() << std::endl;
        semaphore.post();
        return; // Indicate failure
      }
    }

    return; // Success
  }

  bool load(std::vector<Record> &records) {
    // Receive the file name for the worker to process
    std::string fileToProcess = recieveString();

    /// Remove the null terminator
    if (!fileToProcess.empty() && fileToProcess.back() == '\0') {
      fileToProcess.pop_back();
    }

    // Terminating condition
    if (fileToProcess == "End of Work") {
      std::cout << "Worker " << rank << " is terminating" << std::endl;
      return false;
    }

    records = Loader::loadFile(fileToProcess);
    return true;
  }

  void run() {
    loadBaseLine();

    while (true) {

      // Load the file
      std::vector<Record> records;
      bool loaded = load(records);
      if (!loaded)
        break;

      Results results;
      results.date = records[0].date;

      // Calculate the average AQI
      results.aqi = calculateAQIStats(records);
      results.concentration = calculateConcentrationStats(records);
      results.parameter_stats = calculateParameterStats(records);

      std::vector<Record> baseline = getBaselineRecords(records[0].date);
      results.baseline_stats = calculateAQIStats(baseline);

      // Send the results back to the master
      std::string resultsJson = results.toJson();
      std::vector<unsigned char> buffer(resultsJson.begin(), resultsJson.end());
      MPI_Send(buffer.data(), buffer.size(), MPI_UNSIGNED_CHAR, 0, 0,
               MPI_COMM_WORLD);
    }
    boost::interprocess::shared_memory_object::remove("BaselineData");
  }

  // Receive the file name for the worker to process
  void receiveData() {
    std::cout << "Worker " << rank << " is receiving data" << std::endl;
  }

  // Get the baseline records with the same hour as the input record
  std::vector<Record> getBaselineRecords(std::string date) {
    std::vector<Record> records;
    for (auto &[record_date, records] : baselineRecords) {
      // Check if the dates (format: YYYY-MM-DDTHH:MM) have the same hour
      // if the last 5 characters are the same, they have the same hour
      // Print the two dates to check
      if (date.substr(11, 2) == record_date.substr(11, 2)) {
        return baselineRecords[record_date];
      }
    }
    return records;
  }

  AggregateStats calculateAQIStats(std::vector<Record> records) {
    omp_set_num_threads(4);

    int count = 0;
    int sum = 0;
    int max = INT_MIN;
    int min = INT_MAX;

#pragma omp parallel for reduction(+ : count, sum) reduction(max:max) reduction(min:min)
    for (unsigned int i = 0; i < records.size(); i++) {
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
    for (unsigned int i = 0; i < records.size(); i++) {
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