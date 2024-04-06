//#include <mpi.h>
#include "loader.hpp"
#include "mpi.h"
#include "nodes.hpp"
#include <algorithm> // For std::sort
#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace std;
using namespace std::chrono;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    throw std::invalid_argument(
        "Invalid number of args to main. main base_input_dir");
  }

  int world_rank, world_size;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  if (world_rank == MASTER_RANK) {
    // Master node logic
    MasterNode master(world_size, argv[1], "20200814-01", "20200924-23");
    master.run();

  } else {
    // Worker node logic
    WorkerNode worker(world_rank, world_size, argv[1]);
    worker.run();
  }

  // Synchronize all processes here before finalizing
  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Finalize();
  return 0;
}