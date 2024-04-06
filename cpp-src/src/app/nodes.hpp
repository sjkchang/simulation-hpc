// #include <mpi.h>
#include "loader.hpp"
#include "mpi.h"
#include <algorithm> // For std::sort
#include <chrono>
#include <cmath> // Include cmath for M_PI
#include <filesystem>
#include <iostream>
#include <sstream> // std::stringstream
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

#define MASTER_RANK 0
#define END_OF_WORK "END"

const int TERMINATION_TAG = 999;

// 1000 ms = 1 hour of simulation time
const int stepSize = 100;

enum Operation { AVGERAGE = 1, STATISTICS = 2 };

class Node {
public:
  int world_rank;
  int world_size;
  Node(int world_rank, int world_size)
      : world_rank(world_rank), world_size(world_size) {}
  virtual void run() = 0;
  virtual ~Node() {}
};

enum Tags {
  DATA_SIZE,
  DATE_TAG,
  STATS_TAG,
};

struct Results {
  int minAQI = 999;
  int maxAQI = 0;
  int aqiSum = 0;
  int aqiCount = 0;
  float averageAQI;

  int minConcentration;
  int maxConcentration;
  int concentrationSum;
  int concentrationCount;
  float averageConcentration;

  int numHealthAlerts;
};

class MasterNode : public Node {
  std::string base_dir;
  std::string start_date;
  std::string end_date;

  int numSent = 0;
  int current_worker_rank = 1;

  Loader loader;

  std::map<std::string, Results> results;

  int aqiSum = 0;
  int aqiCount = 0;
  int maxAQI = 0;

public:
  MasterNode(int world_size, std::string base_dir, std::string start_date,
             std::string end_date)
      : Node(0, world_size), base_dir(base_dir), start_date(start_date),
        end_date(end_date) {}

  void run() {
    distributeData();
    receiveData();
    terminateWorkers();

    runSimulation();
  }

  void runSimulation() {
    int stepSize = 100;
    // Loop through the results and print them
    bool firstIteration = true;
    for (auto [date, result] : results) {
      aqiSum += result.aqiSum;
      aqiCount += result.aqiCount;
      if (result.maxAQI > maxAQI) {
        maxAQI = result.maxAQI;
      }

      if (result.aqiCount == 0) {
        continue;
      }
      auto start_time = std::chrono::high_resolution_clock::now();

      // Move currsor up to print the next result

      std::cout << "Date: " << date << std::endl;
      std::cout << "Running Max AQI: " << maxAQI << std::endl;
      std::cout << "Running Average AQI: " << aqiSum / aqiCount << std::endl;
      std::cout << "Daily Min AQI: " << result.minAQI << std::endl;
      std::cout << "Daily Max AQI: " << result.maxAQI << std::endl;
      std::cout << "Daily Average AQI: " << result.averageAQI << std::endl;
      std::cout << "Daily Min Concentration: " << result.minConcentration
                << std::endl;
      std::cout << "Daily Max Concentration: " << result.maxConcentration
                << std::endl;
      std::cout << "Daily Average Concentration: "
                << result.averageConcentration << std::endl;
      std::cout << "Daily Number of Health Alerts: " << result.numHealthAlerts
                << std::endl;

      auto end_time = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                          end_time - start_time)
                          .count();

      // Dont run if last iteration
      if (date.compare("2020-09-24T23:00") == 0) {
        continue;
      }
      std::cout << "\033[10A";

      // Sleep for the remaining time
      if (duration < stepSize) {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(stepSize - duration));
      }
    }
  }

  void distributeData() {
    FileIterator fileIterator(base_dir, start_date, end_date);
    std::vector<std::string> filePaths = fileIterator.getAllFileNames();

    int filesPerWorker = filePaths.size() / (world_size - 1);
    int remainingFiles = filePaths.size() % (world_size - 1);

    for (int currentWorker = 1; currentWorker < world_size; ++currentWorker) {
      int start_index = (currentWorker - 1) * filesPerWorker;
      int end_index = start_index + filesPerWorker - 1;
      if (currentWorker == world_size - 1) {
        end_index += remainingFiles;
      }

      char start_file[16];
      strcpy(start_file, filePaths[start_index].c_str());
      MPI_Send(&start_file, 16, MPI_CHAR, currentWorker, 0, MPI_COMM_WORLD);

      char end_file[16];
      strcpy(end_file, filePaths[end_index].c_str());
      MPI_Send(&end_file, 16, MPI_CHAR, currentWorker, 0, MPI_COMM_WORLD);
    }
    numSent = filePaths.size() - 4;
  }

  void receiveData() {
    for (int i = 0; i < 500; ++i) {
      // Receive the date
      MPI_Status status;
      char date[17];
      MPI_Recv(&date, 17, MPI_CHAR, MPI_ANY_SOURCE, Tags::DATE_TAG,
               MPI_COMM_WORLD, &status);

      // Receive the data
      int data[9];
      MPI_Recv(&data, 9, MPI_INT, status.MPI_SOURCE, Tags::STATS_TAG,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      Results result;
      result.minAQI = data[0];
      result.maxAQI = data[1];
      result.aqiSum = data[2];
      result.aqiCount = data[3];
      result.averageAQI = data[2] / data[3];
      result.minConcentration = data[4];
      result.maxConcentration = data[5];
      result.concentrationSum = data[6];
      result.concentrationCount = data[7];
      result.averageConcentration = data[6] / data[7];
      result.numHealthAlerts = data[8];

      results[date] = result;
    }
  }

  void terminateWorkers() {
    for (int i = 1; i < world_size; ++i) {
      int data = 0;
      MPI_Send(&data, 1, MPI_INT, i, TERMINATION_TAG, MPI_COMM_WORLD);
    }
  }
};

std::vector<std::string> healthAlerts;

class WorkerNode : public Node {
private:
  Loader loader;
  std::string base_dir;

public:
  WorkerNode(int world_rank, int world_size, std::string base_dir)
      : Node(world_rank, world_size), base_dir(base_dir) {}

  void run() {
    char start_file[16];
    MPI_Recv(&start_file, 16, MPI_CHAR, 0, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);

    char end_file[16];
    MPI_Recv(&end_file, 16, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    loader.loadFiles(base_dir, "20200810-01", "20200810-23");
    loader.loadFiles(base_dir, start_file, end_file);

    calculateStats();
    // generateHealthAlerts();
  }

  void generateHealthAlerts() {
    for (auto [date, records] : loader.date_records) {

      for (auto record : records) {
        if (record->category > 4) {
        }
      }
    }
  }

  double calculateDistance(double lat, double lon) {
    // 39.765°N -122.673°E - Location of the complex fires
    double lat1 = 39.765;
    double lon1 = -122.673;

    // Convert degrees to radians
    double lat1Rad = lat1 * M_PI / 180.0;
    double lon1Rad = lon1 * M_PI / 180.0;
    double lat2Rad = lat * M_PI / 180.0;
    double lon2Rad = lon * M_PI / 180.0;

    // Calculate differences
    double deltaLat = lat2Rad - lat1Rad;
    double deltaLon = lon2Rad - lon1Rad;

    // Calculate distance without considering Earth's curvature
    // This is a simplification and not suitable for long distances
    double distance = sqrt(deltaLat * deltaLat + deltaLon * deltaLon);

    return distance;
  }

  void calculateStats(int distanceFilter = 0) {

    for (auto [date, records] : loader.date_records) {
      // Skip if date starts with 2020-08-10
      if (date.compare(0, 10, "2020-08-10") == 0) {
        continue;
      }

      int aqiSum = 0;
      int aqiCount = 0;
      int minAQI = 999;
      int maxAQI = 0;
      int concentrationSum = 0;
      int concentrationCount = 0;
      int minConcentration = 999;
      int maxConcentration = 0;

      int numHealthAlerts = 0;

      for (auto record : records) {

        if (distanceFilter > 0) {
          double distance =
              calculateDistance(record->latitude, record->longitude);

          if (distance > distanceFilter) {
            continue;
          }
        }

        if (record->aqi != -999) {
          aqiSum += record->aqi;
          aqiCount++;
          if (record->aqi < minAQI) {
            minAQI = record->aqi;
          }
          if (record->aqi > maxAQI) {
            maxAQI = record->aqi;
          }
        }
        if (record->concentration != -999) {
          concentrationSum += record->concentration;
          concentrationCount++;
          if (record->concentration < minConcentration) {
            minConcentration = record->concentration;
          }
          if (record->concentration > maxConcentration) {
            maxConcentration = record->concentration;
          }
        }

        if (record->category > 4) {
          numHealthAlerts++;
        }
      }
      int data[9] = {minAQI,           maxAQI,
                     aqiSum,           aqiCount,
                     minConcentration, maxConcentration,
                     concentrationSum, concentrationCount,
                     numHealthAlerts};

      char date_char[17];
      strcpy(date_char, date.c_str());
      MPI_Send(&date_char, 17, MPI_CHAR, 0, Tags::DATE_TAG, MPI_COMM_WORLD);

      // Send the data
      MPI_Send(&data, 9, MPI_INT, 0, Tags::STATS_TAG, MPI_COMM_WORLD);
    }
  }
};
