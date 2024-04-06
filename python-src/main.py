from mpi4py import MPI

def main():
    comm = MPI.COMM_WORLD
    size = comm.Get_size()  # Total number of processes
    rank = comm.Get_rank()  # The rank of the current process

    print(f"Hello from the process {rank} out of {size}")

if __name__ == "__main__":
    main()