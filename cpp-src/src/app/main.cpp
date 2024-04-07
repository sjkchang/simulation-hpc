#include "airnow/record.hpp"
#include "loader.hpp"
#include "mpi.h"
#include "nodes.hpp"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <fstream>
#include <iostream>

int main(int argc, char *argv[]) {
  // Setup MPI
  int world_rank;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  // Print the world rank

  // Master node should be impelemented in python
  if (world_rank == 0) {
    return -1;
  }
  Worker worker(world_rank, 0);
  worker.run();

  MPI_Finalize();
  return 0;
}
