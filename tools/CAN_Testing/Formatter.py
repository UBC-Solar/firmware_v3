from Prompter import Prompter
from FileUtils import getValidLines
from FileUtils import getDBCFile
from FileUtils import find_file
import os

""" DEFINE NAMING CONSTANTS """
INFLUX_FILE_NAME = "influx_log.txt"
prompt = Prompter()
DBC_FILE_PATH = getDBCFile()


"""
The Influx DB Formatter class will take log messages from PCAN view
and will convert their format to the format for our Influx DB. Format is:

------------------------------------------------------------
CAN:
ID: 0x800, Data: 0x8324, Timetamp: 28492, Length: 8

GPS:
Lat: 38.2134, Lon: 91.1234, Sat: 18, Alt: 2134.324

IMU:
Imu_type: 122, dim: 12, data: 0x32A5
------------------------------------------------------------

Note that we only care about the CAN messages currently. 
Note that timestamp is in miliseconds
"""
class InfluxDBFormatter:
    def __init__(self, log_file, json_file, displayPassed):
        # Needs both files names to use the AutoChecker class
        self.log_file_name = log_file
        self.json_file_name = json_file
        self.displayPassed = displayPassed


    """
    Format a CAN message to the Influx DB format
    Write it to a file called "influx_log.txt"

    RETURNS: None
    """
    def formatLogToInflux(self):
        # Try to open influx file to write
        dir_of_dbc = os.path.dirname(DBC_FILE_PATH)
        influx_file_path = os.path.join(dir_of_dbc, INFLUX_FILE_NAME)
        try:
            with open(influx_file_path, "w") as influx_file:
                # Use get valid lines from Reciever
                for line in getValidLines(self.log_file_name, "Rx"):
                    lineAsList = [entry for entry in line.split() if entry]
                    can_message = self.formatCAN(lineAsList)
                    influx_file.write(can_message)

                # Close the files
                influx_file.close()

        except Exception as e:
            print(f"Error creating influx log file: {e}")

    
    """
    Format a single log line to the Influx DB format

    PARAMS:
        split_line: list of strings in one log line. Ex. ['1)', '0.0', 'Rx', '0701', '8', 'CC', '00', 'F0', '40', 'EE', '00', '90', '40']
    
    RETURNS: string of the CAN message in the Influx DB format
    """
    def formatCAN(self, split_line):
        # Get the ID, Data, and Timestamp and length
        id = "0x" + split_line[3].lstrip("0")
        data = "0x" + ''.join(split_line[5:])
        timestamp = split_line[1]
        length = split_line[4]

        # Format the CAN message
        can_message = "ID: " + id + ", Data: " + data + ", Timestamp: " + timestamp + ", Length: " + length + "\n"

        return can_message
    