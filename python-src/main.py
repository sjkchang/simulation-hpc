import json
import time
from mpi4py import MPI
import numpy as np
import glob


def getFiles(directory_path):
    # Pattern to match files in the format data/YYYYMMDD/YYYYMMDD-HH.csv
    pattern = f"{directory_path}/*/*-??.csv"
    
    # Use glob to find all files that match the pattern
    files = glob.glob(pattern)

    # Sort the list of files
    sorted_files = sorted(files)
    return sorted_files
    
def encode_string(str):
    data_bytes = str.encode('utf-8')  # Encode string to bytes
    data_np = np.frombuffer(data_bytes, dtype='B')  # 'B' for unsigned byte
    return data_np

def distribute_files(comm):
    # List all files in the data directory
    files = getFiles("data")
    
    current_worker = 1
    for file in files:
        comm.Send([encode_string(file), MPI.CHAR], dest=current_worker, tag=0)  ####################### CONVERT TO NON-BLOCKING
        current_worker += 1
        if current_worker == comm.Get_size():
            current_worker = 1

    # Sleep for 1 second to allow workers to process the files
    time.sleep(1)

    # Send End of Work signal to all workers
    for worker in range(1, comm.Get_size()):
        comm.Send([encode_string("End of Work"), MPI.CHAR], dest=worker, tag=0)

    return len(files)

def recieve_string(comm, source = MPI.ANY_SOURCE, tag=MPI.ANY_TAG):
    status = MPI.Status()
    comm.Probe(source=MPI.ANY_SOURCE, tag=MPI.ANY_TAG, status=status)
    message_size = status.Get_count(MPI.UNSIGNED_CHAR)

    # Allocate a bytearray of the appropriate size
    data = bytearray(message_size)
    
    # Receive the message into the bytearray
    comm.Recv([data, MPI.UNSIGNED_CHAR], source=status.Get_source(), tag=status.Get_tag())

    # Convert the bytearray to a string (assuming UTF-8 encoding)
    received_str = data.decode('utf-8')
    return received_str

comm = MPI.COMM_WORLD
rank = comm.Get_rank()

if rank == 0:
    numFilesSent = distribute_files(comm)
    
    results_dict = {}
    # Receive the results from the workers
    while(numFilesSent > 0):
        received_str = recieve_string(comm)
        print(received_str)
        numFilesSent -= 1

        results = json.loads(received_str)
        results_dict[results['date']] = results
    
    seconds_per_hour = 0.05
    # Print the results
    for key, results in sorted(results_dict.items()):
        # Measure time it takes to print the results
        start_time = time.time()

        print("\033[2J\033[H", end="")
        print("Date: ", key)
        print("Cumulative AQI: ", results["aqi"]["mean"])
        print("Cumulative Max AQI: ", results["aqi"]["max"])
        print("Cumulative Min AQI: ", results["aqi"]["min"])

        print("Cumulative Average Concentration: ",  results["concentration"]["mean"])
        print("Cumulative Max Concentration: ", results["concentration"]["max"])
        print("Cumulative Min Concentration: ", results["concentration"]["min"])



        # Measure time it takes to print the results
        elapsed_time = time.time() - start_time
        if(elapsed_time < seconds_per_hour):
            sleep_time = seconds_per_hour - elapsed_time
            time.sleep(sleep_time)
    
# Finalize the MPI environment
MPI.Finalize()
