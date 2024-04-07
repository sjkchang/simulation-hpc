import json
import time
from mpi4py import MPI
import numpy as np
import glob

class Record:
    def __init__(self, latitude, longitude, date, parameter, concentration, unit, rawConcentration, aqi, category, siteName, siteAgency, siteId):
        self.latitude = latitude
        self.longitude = longitude
        self.date = date
        self.parameter = parameter
        self.concentration = concentration
        self.unit = unit
        self.rawConcentration = rawConcentration
        self.aqi = aqi
        self.category = category
        self.siteName = siteName
        self.siteAgency = siteAgency
        self.siteId = siteId

    # COnvert json string to Record object
    @staticmethod
    def from_json(json_string):
        record = json.loads(json_string)
        # Print the keys of the record
        print(record.keys())
        return Record(record['lat'], record['lon'], record['date'], record['parameter'], record['concentration'], record['unit'], record['rawConcentration'], record['aqi'], record['category'], record['siteName'], record['siteAgency'], record['siteId'])
    
    @staticmethod
    def from_dict(dict):
        return Record(dict['lat'], dict['lon'], dict['date'], dict['parameter'], dict['concentration'], dict['unit'], dict['rawConcentration'], dict['aqi'], dict['category'], dict['siteName'], dict['siteAgency'], dict['siteId'])

    # Convert Json string with list of records to list of Record objects
    @staticmethod
    def from_json_list(json_string):
        records_json = json.loads(json_string)
        records = records_json['records']
        return [Record.from_dict(record) for record in records]

    # To string method
    def __str__(self):
        return f"Record({self.latitude}, {self.longitude}, {self.date}, {self.parameter}, {self.concentration}, {self.unit}, {self.rawConcentration}, {self.aqi}, {self.category}, {self.siteName}, {self.siteAgency}, {self.siteId})"

def list_sorted_files(directory_path):
    # Pattern to match files in the format data/YYYYMMDD/YYYYMMDD-HH.csv
    pattern = f"{directory_path}/*/*-??.csv"
    
    # Use glob to find all files that match the pattern
    files = glob.glob(pattern)

    # Strip the directory path and subdirectory from the filenames
    files = [file.split("/")[-1] for file in files]

    # Strip the files starting with 20200810
    files = [file for file in files if not file.startswith("20200810")]

    # Sort the list of files
    sorted_files = sorted(files)
    print(sorted_files)
    return sorted_files
    
def encode_string(str):
    data_bytes = str.encode('utf-8')  # Encode string to bytes
    data_np = np.frombuffer(data_bytes, dtype='B')  # 'B' for unsigned byte
    return data_np

def distribute_files(comm):
    # List all files in the data directory
    files = list_sorted_files("data")
    data = "Hello from Python!"  # String to send
    
    # List all files in the data directory
    files = list_sorted_files("data")
    

    files_per_process = len(files) // (comm.Get_size() - 1)
    remainder_files = len(files) % (comm.Get_size() - 1)

    # Send start filepaths and end filepaths to each process
    # They will read the files in between
    for current_worker in range(1, comm.Get_size() ):
        start_index = (current_worker - 1) * files_per_process
        if current_worker == comm.Get_size() - 1:
            print(f"Adding remainder files: {remainder_files} to the last worker")
            end_index = start_index + files_per_process - 1 + remainder_files
        else:
            end_index = start_index + files_per_process - 1


        startFile = files[start_index]
        endFile = files[end_index]
        # Print the start and end files
        print("Py Start File: ", startFile)
        print("Py End File: ", endFile)


        # Send the byte array to process 1
        print("Sending Start and End Files to Worker: ", current_worker)
        comm.Send([encode_string(startFile), MPI.CHAR], dest=current_worker, tag=0)
        comm.Send([encode_string(endFile), MPI.CHAR], dest=current_worker, tag=0)

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

    numResultsReceived = 0
    # Receive results from each worker
    for file in range(0, numFilesSent):
        result_string = recieve_string(comm, source=MPI.ANY_SOURCE, tag=0)
        numResultsReceived += 1
        #records = Record.from_json_list(records_string)
        # Get JSon object from the string
        results = json.loads(result_string)
        print(results)

        # Add the results to the results dictionary
        results_dict[results['date']] = results



    aqiSum = 0
    aqiCount = 0
    concentrationSum = 0
    concentrationCount = 0
    maxAQI = 0
    minAQI = 1000
    maxConcentration = 0
    minConcentration = 1000

    seconds_per_hour = 0.05
    # Print the results
    for key, value in sorted(results_dict.items()):
        # Measure time it takes to print the results
        start_time = time.time()

        aqiSum += int(value['compiledStatistics']["aqiSum"])
        aqiCount += int(value['compiledStatistics']["aqiCount"])
        concentrationSum += int(value['compiledStatistics']["concentrationSum"])
        concentrationCount += int(value['compiledStatistics']["concentrationCount"])
        maxAQI = max(maxAQI, int(value['compiledStatistics']["maxAQI"]))
        minAQI = min(minAQI, int(value['compiledStatistics']["minAQI"]))
        maxConcentration = max(maxConcentration, int(value['compiledStatistics']["maxConcentration"]))
        minConcentration = min(minConcentration, int(value['compiledStatistics']["minConcentration"]))


        print("\033[2J\033[H", end="")
        print("Date: ", key)
        print("Cumulative AQI: ", aqiSum / aqiCount)
        print("Cumulative Max AQI: ", maxAQI)
        print("Cumulative Min AQI: ", minAQI)
        print("Cumulative Average Concentration: ", concentrationSum / concentrationCount)
        print("Cumulative Max Concentration: ", maxConcentration)
        print("Cumulative Min Concentration: ", minConcentration)
        print("Hourly Average AQI: ", int(value['compiledStatistics']["aqiSum"]) / int(value['compiledStatistics']["aqiCount"]))
        print("Hourly Max AQI: ", value['compiledStatistics']["maxAQI"])
        print("Hourly Min AQI: ", value['compiledStatistics']["minAQI"])
        print("Hourly Average Concentration: ", int(value['compiledStatistics']["concentrationSum"]) / int(value['compiledStatistics']["concentrationCount"]))
        print("Hourly Max Concentration: ", value['compiledStatistics']["maxConcentration"])
        print("Hourly Min Concentration: ", value['compiledStatistics']["minConcentration"])

        # Measure time it takes to print the results
        elapsed_time = time.time() - start_time
        if(elapsed_time < seconds_per_hour):
            sleep_time = seconds_per_hour - elapsed_time
            time.sleep(sleep_time)

    
