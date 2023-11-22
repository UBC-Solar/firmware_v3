from FileUtils import getValidLines
import numpy as np

""" CONSTANTS """
INTERVAL_MS                         = 1 
MAX_CAN_LOAD_BYTES_PER_S            = 5 * 1000000           # 5 MB/s
MAX_CAN_LOAD_INTERVAL_BYTES         = MAX_CAN_LOAD_BYTES_PER_S * (INTERVAL_MS / 1000)
T_BIT                               = 1.0 / (1000000 * 8)


class JitterAnalyzer:
    def __init__(self, log_file):
        # Need just the log file to determine CAN LOAD
        self.log_file_name = log_file


    """
    Print the Max and Min CAN LOAD in the interval as a percentage of the MAX

    RETURNS: None
    """
    def displayMaxMinCANLoad(self):
        # Get the max and min bytes in the interval
        max_bytes, min_bytes = self.getMaxMinBytesInInterval()

        # Calculate the CAN LOAD as a percentage of the MAX
        max_can_load_percent = (max_bytes / MAX_CAN_LOAD_INTERVAL_BYTES) * 100
        min_can_load_percent = (min_bytes / MAX_CAN_LOAD_INTERVAL_BYTES) * 100

        # Print the CAN LOAD
        print(f"Max CAN LOAD for {INTERVAL_MS} ms is {round(max_can_load_percent, 2)}%: \n"
              f"    {round((max_bytes / INTERVAL_MS) * 1000, 2)} bytes/s of MAX ({MAX_CAN_LOAD_BYTES_PER_S} bytes/s)")
        print()
        print(f"Min CAN LOAD for {INTERVAL_MS} ms is {round(min_can_load_percent, 2)}%: \n"
              f"    {round((min_bytes / INTERVAL_MS) * 1000, 2)} bytes/s of MAX ({MAX_CAN_LOAD_BYTES_PER_S} bytes/s)")
        
        print()


    """
    Get the Max and Min bytes in the interval from the log file

    RETURNS: tuple of the max and min bytes in the interval
    """
    def getMaxMinBytesInInterval(self):
        # Try to open log file
        try:
            log_file = open(self.log_file_name, "r")
        except:
            print("Error opening log file.")
            return
        
        # Get a list of all the valid lines
        allValidLines = getValidLines(log_file, "All")

        # Loop through lines in groups of INTERVAL until the end
        start_idx = 0
        last_idx = self.getLastIndexOfInterval(start_idx, allValidLines)
        max_bytes = 0
        min_bytes = 0
        while last_idx < len(allValidLines):
            # Get the sum of the data lengths in the interval
            total_bytes = np.sum([int(line.split()[4]) for line in allValidLines[start_idx : last_idx + 1]])

            # Track the max and min
            max_bytes = max(max_bytes, total_bytes)
            min_bytes = min(min_bytes, total_bytes)

            # Get the next interval
            start_idx += 1

            # If the current last index is the last line, break
            if last_idx == len(allValidLines) - 1:
                break

            last_idx = self.getLastIndexOfInterval(start_idx, allValidLines)
    
        # Close the file
        log_file.close()

        # return the max and min bytes as a tuple
        return max_bytes, min_bytes


    """
    Get the last line to check before the timestamp is past the interval

    PARAMS:
        allValidLines:  list of all the valid lines to check

    RETURNS: the index of the last line before timestamp is beyond the interval
    """
    def getLastIndexOfInterval(self, startIndex, allValidLines):
        # Go through each line and check if the timestamp is within the interval
        for i in range(startIndex, len(allValidLines)):
            # If the current line is the last line, return its index
            if i == len(allValidLines) - 1:
                return i
            curr_line = allValidLines[i]
            start_line = allValidLines[startIndex]
            curr_timestamp = float(curr_line.split()[1])
            if curr_timestamp - float(start_line.split()[1]) > INTERVAL_MS:
                return i - 1
        