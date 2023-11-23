from FileUtils import getValidLines
import numpy as np

# TODO: How big interval??

""" CONSTANTS """
INTERVAL_MS                         = 1                             # 1 ms for "exampleLog.txt", normally 1000 
MAX_CAN_LOAD_BYTES_PER_S            = 500 * pow(2, 10) / 8          # 500 kilo-bit/s = 64000 bytes
MAX_CAN_LOAD_INTERVAL_BYTES         = MAX_CAN_LOAD_BYTES_PER_S * (INTERVAL_MS / 1000)
T_BIT                               = 1.0 / (1000000 * 8)


class JitterAnalyzer:
    def __init__(self, log_file, failedSet):
        # Need just the log file to determine max CAN LOAD
        self.log_file_name = log_file

        # A list of message numbers which exceeded alotted interval with delta
        self.failedSet = failedSet

        # All the valid lines
        self.allValidLines = getValidLines(log_file, "All")


    """
    This method provides the CAN load report which includes
    the maximum observed CAN load in the interval 
    and the CAN load at the time of an out of interval failure
    
    RETURNS: None
    """
    def displayCANLoadReport(self):
        # Show the Max CAN Load (min not used currently)
        print("\n------------------------- MAX CAN LOAD --------------------------\n")
        self.displayMaxMinCANLoad()

        # Show the CAN Load at the time of the problem
        print("\n------------------ CAN LOAD AT FAILED MESSAGES ------------------\n")
        self.displayTimeOfFailureCANLoad()

    
    """
    Print the CAN LOAD at the time of failure for each message number
    from the failed list
    
    RETURNS: None
    """
    def displayTimeOfFailureCANLoad(self):
        # Loop through failed messages
        for failedMessageNum in self.failedSet:
            # Find the index of this message num in valid lines
            start_index, timestamp = self.getIndexOfMessageNum(failedMessageNum)

            # Get the last index to check before the timestamp is past the interval
            last_index = self.getLastIndexOfInterval(start_index, mode="Past")

            # Get the sum of the data lengths in the interval
            total_bytes = np.sum([int(line.split()[4]) for line in self.allValidLines[last_index : start_index + 1]])

            # Calculate the CAN LOAD as a percentage of the MAX
            can_load_percent = (total_bytes / MAX_CAN_LOAD_INTERVAL_BYTES) * 100

            # Display the CAN LOAD
            print(f"CAN Load at time {timestamp} ms for message {failedMessageNum} is {round(can_load_percent, 2)}%: \n"
                  f"    {round((total_bytes / INTERVAL_MS) * 1000, 2)} bytes/s of MAX ({MAX_CAN_LOAD_BYTES_PER_S} bytes/s)")
            print()

    """
    Get the index of the given message number in the valid lines of the log
    
    PARAMS:
        messageNum:     the message number to find the index of
        
    RETURNS: the index of the message number and the line as a tuple

    """
    def getIndexOfMessageNum(self, messageNum):
        # Loop through valid lines
        for i in range(len(self.allValidLines)):
            # If the message num is found, return the index
            if int(self.allValidLines[i].split()[0].rstrip(")")) == messageNum:
                return i, float(self.allValidLines[i].split()[1])
        
        # If not found, return -1 and -1
        return -1, -1


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
        
        # print()
        # print(f"Min CAN LOAD for {INTERVAL_MS} ms is {round(min_can_load_percent, 2)}%: \n"
        #       f"    {round((min_bytes / INTERVAL_MS) * 1000, 2)} bytes/s of MAX ({MAX_CAN_LOAD_BYTES_PER_S} bytes/s)")
        
        print()


    """
    Get the Max and Min bytes in the interval from the log file

    RETURNS: tuple of the max and min bytes in the interval
    """
    def getMaxMinBytesInInterval(self):
        # Loop through lines in groups of INTERVAL until the end
        start_idx = 0
        last_idx = self.getLastIndexOfInterval(start_idx, mode="Future")
        max_bytes = 0
        min_bytes = 0
        while last_idx < len(self.allValidLines):
            # Get the sum of the data lengths in the interval
            total_bytes = np.sum([int(line.split()[4]) for line in self.allValidLines[start_idx : last_idx + 1]])

            # Track the max and min
            max_bytes = max(max_bytes, total_bytes)
            min_bytes = min(min_bytes, total_bytes)

            # Get the next interval
            start_idx += 1

            # If the current last index is the last line, break
            if last_idx == len(self.allValidLines) - 1:
                break

            last_idx = self.getLastIndexOfInterval(start_idx, mode="Future")
    
        # return the max and min bytes as a tuple
        return max_bytes, min_bytes


    """
    Get the last line to check before the timestamp is past the interval

    PARAMS:
        allValidLines:  list of all the valid lines to check

    RETURNS: the index of the last line before timestamp is beyond the interval
    """
    def getLastIndexOfInterval(self, startIndex, mode):
        # Go through each line and check if the timestamp is within the interval
        if mode == "Future":
            rangeHolder = range(startIndex, len(self.allValidLines))
        elif mode == "Past":
            rangeHolder = range(startIndex, -1, -1)

        for i in rangeHolder:
            # If the current line is the last line or the first line, return its index
            if i == len(self.allValidLines) - 1 and mode == "Future":
                return i
            elif i == -1 and mode == "Past":
                return i + 1
            
            # Get the current and start time stamp and compare
            curr_line = self.allValidLines[i]
            start_line = self.allValidLines[startIndex]
            curr_timestamp = float(curr_line.split()[1])
            if abs(curr_timestamp - float(start_line.split()[1])) > INTERVAL_MS:
                if mode == "Future":
                    return i - 1
                else:
                    return i + 1
        