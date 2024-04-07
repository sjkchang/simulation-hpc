#!/bin/bash

# Run the MPI application with 1 Python master and N-1 C++ workers
# Adjust the number of processes (-n) as needed

mpiexec -n 1 python3 ./python-src/main.py : -n 3 ./cpp-src/b/bin/main