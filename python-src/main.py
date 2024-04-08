import json
import time
from mpi4py import MPI
import glob


def getFiles(directory_path):
    # Pattern to match files in the format data/YYYYMMDD/YYYYMMDD-HH.csv
    pattern = f"{directory_path}/*/*-??.csv"
    
    # Use glob to find all files that match the pattern
    files = glob.glob(pattern)

    # Sort the list of files
    sorted_files = sorted(files)
    return sorted_files

def distribute_files(comm):
    # List all files in the data directory
    files = getFiles("data")
    
    current_worker = 1
    for file in files:
        comm.Send([file.encode('utf-8'), MPI.CHAR], dest=current_worker, tag=0)  ####################### CONVERT TO NON-BLOCKING
        current_worker += 1
        if current_worker == comm.Get_size():
            current_worker = 1

    # Sleep for 1 second to allow workers to process the files
    time.sleep(1)

    # Send End of Work signal to all workers
    for worker in range(1, comm.Get_size()):
        comm.Send(["End of Work".encode('utf-8'), MPI.CHAR], dest=worker, tag=0)

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
        numFilesSent -= 1

        results = json.loads(received_str)
        results_dict[results['date']] = results

  

    # Calculate the average AQI and concentration for the entire dataset
    cumulative_sum_aqi = 0
    cumulative_count_aqi = 0
    cumulative_sum_concentration = 0
    cumulative_count_concentration = 0
    cumulative_max_aqi = 0
    cumulative_max_concentration = 0

    seconds_per_hour = 0.05
    # Print the results
    for key, results in sorted(results_dict.items()):
        # Measure time it takes to print the results
        start_time = time.time()

        cumulative_sum_aqi += int(results["aqi"]["sum"])
        cumulative_count_aqi += int(results["aqi"]["count"])
        cumulative_max_aqi = max(cumulative_max_aqi, int(results["aqi"]["max"]))
        cumulative_sum_concentration += int(results["concentration"]["sum"])
        cumulative_count_concentration += int(results["concentration"]["count"])
        cumulative_max_concentration = max(cumulative_max_concentration, int(results["concentration"]["max"]))

        aqi_delta = float(results["aqi"]["mean"]) - float(results["baseline_aqi"]["mean"])

        
        print("\033[2J\033[H", end="")
        print("Current Time: ", key)

        print("Current Statistics: ")
        print("Avg. AQI: ", results["aqi"]["mean"])
        print("Change from Baseline: ", aqi_delta)
        print("Max AQI: ", results["aqi"]["max"])
        print("Min AQI: ", results["aqi"]["min"])
        print("Avg. Concentration: ",  results["concentration"]["mean"])
        print("Max Concentration: ", results["concentration"]["max"])
        print("Min Concentration: ", results["concentration"]["min"])

        print()
        print("Cumulative Statistics: ")
        print("Avg. AQI: ", cumulative_sum_aqi / cumulative_count_aqi)
        print("Max AQI: ", cumulative_max_aqi)
        print("Avg. Concentration: ",  cumulative_sum_concentration / cumulative_count_concentration)
        print("Max Concentration: ", cumulative_max_concentration)


        # Measure time it takes to print the results
        elapsed_time = time.time() - start_time
        if(elapsed_time < seconds_per_hour):
            sleep_time = seconds_per_hour - elapsed_time
            time.sleep(sleep_time)
   
    
    
    
# Finalize the MPI environment
MPI.Finalize()
