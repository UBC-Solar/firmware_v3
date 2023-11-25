import cantools
import json
from FileUtils import getValidLines


""" DEFINE CONSTANTS """
from Parameters import DELTA

# Indexing constants
MIN = 0
MAX = 1
INTERVAL = 2

START_VAL = 0
EXPECTED_VAL = 1


"""
Class of methods related to obtaining and parsing logged messages
and comparing these messages to the JSON requiremnt file
"""
class AutoChecker:
    def __init__(self, dbc_name, log_file, json_file, displayPassed):
        # Set the names of the log, JSON, and DBC files
        self.log_name, self.json_name = log_file, json_file
        self.dbc = self.getDBCasOBJ(dbc_name)

        # Quickyl grab length of signal with dict of signal_name (key) -> length of signal (value)
        self.signalLenToDict = self.getSignalToLengthDict()

        # Use a dictionary structure of signal_name (key) -> time offset of recent signal (value)
        self.signalTimeDifferences = {}

        # Use a dictionary structure of signal_name (key) -> what the current status should be (value)
        self.signalStatuses = {}

        # Get the requirements file as a JSON object/Dictionary
        self.requirements = self.getRequirementsOBJ(self.json_name)

        # Flag to filter PASSED tests
        self.showPassed = displayPassed

        # List to track failed tests with the corresponding message number
        self.failedSet = set()


    """
    Reads the requirements file and returns it as a JSON object

    PARAMS:
        file_name: The name of the requirements file
    
    RETURNS: JSON object of the requirements file
    """
    def getRequirementsOBJ(self, file_name):
        with open(file_name, "r") as read_file:
            data = json.load(read_file)
        return data


    """
    Reads the DBC file and returns it as a cantools database object

    PARAMS:
        dbc_name: The name of the DBC file

    RETURNS: cantools database object of the DBC file
    """
    def getDBCasOBJ(self, dbc_name):
        db = cantools.database.load_file(dbc_name)
        return db


    """
    Takes the log file and goes through each line and compares data
    Utilizes multiple helper functions to print the results

    RETURNS: None
    """
    def checkLog(self):
        # Continuously Read lines and compare data
        for line in getValidLines(self.log_name, type="Rx"):
            self.compareData(line)


    """
    Given a log line of the recieved message
    Go into JSON file and find expected values for that data
    If in range then say passed with the Data value itself and the range
    If not in range then say failed with the Data value itself and the range
    Line is of the form:
        '     1)         0.0  Rx         01C7  7  06 A0 B9 5E 00 00 3E '
    
    PARAMS:
        log_message: The log message to compare to the JSON file

    RETURNS: None
    """
    def compareData(self, log_message):
        # Parse log message and get ID and data
        # ID is n "0" + hex val format
        # Data is decimal values
        timestamp, hex_id, data, messageNum = self.getInfoFromLog(log_message)
        self.checkJSON(timestamp, hex_id, data, messageNum)


    """
    Looks at the JSON for the expected values for a given ID
    Delegates to statusCheck or valueCheck depending on the signal length

    PARAMS:
        timestamp: The timestamp of the message
        hex_id: The ID of the message
        data: The data of the message
        messageNum: The message number of the message based on the log file
    
    RETURNS: None
    """
    def checkJSON(self, timestamp, hex_id, data, messageNum):
        # Get the data from the JSON file
        # Data from log when decoded is a dictionary
        # Loop through size of dict (data) and compare each value from JSON
        for signal, value in data.items():
            # Get the expected data for that signal
            # This is a list of values with min max interval or start_val exp_val interval
            expected_data = self.requirements["Receive"][hex_id][signal]

            # Get the time difference for this signal
            timeSinceLast = expected_data[INTERVAL]
            if signal in self.signalTimeDifferences:
                timeSinceLast = timestamp - self.signalTimeDifferences[signal]

            # Update the latest time in the dict
            self.signalTimeDifferences[signal] = timestamp

            # With the signal name we can find if it is a status or a value
            # We do this by finding what the DBC says the signal length is
            if self.signalLenToDict[signal] <= 2:
                self.statusCheck(
                    hex_id, signal, value, timeSinceLast, expected_data, messageNum
                )
            elif 2 < self.signalLenToDict[signal] <= 64:
                self.valueCheck(
                    hex_id, signal, value, timeSinceLast, expected_data, messageNum
                )
            else:
                print(
                    "Error: Expected data not in correct format. Ensure 0 < bytes of data <= 8"
                )


    """
    Specifically checks the status of the signal compared to the JSON
    If status never changes to expected value, passed is never printed
    Note that start_val and exp_val must be hex or decimal not a mix. Must put "0x" prefix for hex
    Also: Interval is always decimal
    Note: Delta of <code>DELTA<code> is used for interval

    PARAMS:
        hex_id: The ID of the message
        signal: The signal name
        value: The value of the signal
        timeSinceLast: The time since the last message for this ID
        expected_data: The expected data from the JSON
        messageNum: The message number of the message based on the log file
    
    RETURNS: None
    """
    def statusCheck(
        self, hex_id, signal, value, timeSinceLast, expected_data, messageNum
    ):
        # Get the start and expected values
        start_val = 0
        exp_val = 0
        interval = 0

        # Set the start and expected values based on given format of data
        if str(expected_data[START_VAL]).startswith("0x"):
            start_val = self.hexToDec(expected_data[START_VAL])
            exp_val = self.hexToDec(expected_data[EXPECTED_VAL])
        else:
            start_val = expected_data[START_VAL]
            exp_val = expected_data[EXPECTED_VAL]
        interval = expected_data[INTERVAL]

        # Value is initially compared to start. When the signalLengthDict has the signal key for this status,
        # we then wait for value to become exp_val.
        # Once it is exp_val we print passed. If never exp_val passed message never displays
        if signal in self.signalStatuses:
            # Check if the value is the start value
            if value == exp_val and self.showPassed == True:
                print(
                    f"PASSED ---- Message Number: {messageNum}, ID: {hex_id}, Signal: {signal}\n"
                    f"    Received Value: {value}\n"
                    f"    Received Interval: {round(timeSinceLast, 3)} ms\n"
                    f"    Expected Start Value: {start_val}, Expected Value: {exp_val}\n"
                    f"    Expected Interval: {expected_data[INTERVAL]} ms"
                )
                print()
        else:
            self.signalStatuses[signal] = start_val
            # Check if the value is the start value
            if value != start_val:
                print(
                    f"FAILED ---- Wrong Start Value\n"
                    f"    Message Number: {messageNum}, ID: {hex_id}, Signal: {signal}\n"
                    f"    Received Value: {value}\n"
                    f"    Received Interval: {round(timeSinceLast, 3)} ms\n"
                    f"    Expected Start Value: {start_val}, Expected Value: {exp_val}\n"
                    f"    Expected Interval: {expected_data[INTERVAL]} ms"
                )
                print()

        # Make sure it is in the interval
        if not interval - DELTA <= timeSinceLast <= interval + DELTA:
            # Add to failed list
            self.failedSet.add(messageNum)
            print(
                f"FAILED ---- Outside Expected Interval \n"
                f"    Message Number: {messageNum}, ID: {hex_id}, Signal: {signal}\n"
                f"    Received Value: {value}\n"
                f"    Received Interval: {round(timeSinceLast, 3)} ms\n"
                f"    Expected Start Value: {start_val}, Expected Value: {exp_val}\n"
                f"    Expected Interval: {expected_data[INTERVAL]} ms"
            )
            print()


    """
    Compares the data of the specifc ID to the range in the JSON file
    Note that min, max, must be hex or decimal not a mix. Must put "0x" prefix for hex
    Also: Interval is always decimal
    Note: Delta of <code>DELTA<code> is used for interval

    PARAMS:
        hex_id: The ID of the message
        signal: The signal name
        value: The value of the signal
        timeSinceLast: The time since the last message for this ID
        expected_data: The expected data from the JSON
        messageNum: The message number of the message based on the log file

    RETURNS: None
    """
    def valueCheck(
        self, hex_id, signal, value, timeSinceLast, expected_data, messageNum
    ):
        # Get the min and max of the expected data
        min = 0
        max = 0
        interval = 0

        # Set the min, max, and interval based on given format of data
        if str(expected_data[MIN]).startswith("0x"):
            min = self.hexToDec(expected_data[MIN])
            max = self.hexToDec(expected_data[MAX])
        else:
            min = expected_data[MIN]
            max = expected_data[MAX]
        interval = expected_data[INTERVAL]

        # Compare the data to the expected data and a print a message showing differences
        if not interval - DELTA <= timeSinceLast <= interval + DELTA:
            self.failedSet.add(messageNum)
            print(
                f"FAILED ---- Outside Expected Interval \n"
                f"    Message Number: {messageNum}, ID: {hex_id}, Signal: {signal}\n"
                f"    Received Value: {round(value, 3)}\n"
                f"    Received Interval: {round(timeSinceLast, 3)} ms\n"
                f"    Expected Min: {min}, Expected Max: {max}\n"
                f"    Expected Interval: {interval} ms"
            )
            print()
        elif not min <= value <= max:
            print(
                f"FAILED ---- Value Not In Range \n"
                f"    Message Number: {messageNum}, ID: {hex_id}, Signal: {signal}\n"
                f"    Received Value: {round(value, 3)}\n"
                f"    Received Interval: {round(timeSinceLast, 3)} ms\n"
                f"    Expected Min: {min}, Expected Max: {max}\n"
                f"    Expected Interval: {interval} ms"
            )
            print()
        elif self.showPassed == True:
            print(
                f"PASSED ---- Message Number: {messageNum}, ID: {hex_id}, Signal: {signal}\n"
                f"    Received Value: {round(value, 3)}\n"
                f"    Received Interval: {round(timeSinceLast, 3)} ms\n"
                f"    Expected Min: {min}, Expected Max: {max}\n"
                f"    Expected Interval: {interval} ms"
            )
            print()


    """
    Using cantools decode message, the data is turned to decimal based on ID
    Then, the ID and the data and the message number are returned

    PARAMS:
        log_message: The log message to decode

    RETURNS: ID, data, and message number for the log message
    """
    def getInfoFromLog(self, log_message):
        # Get the ID from the log message
        # ID is n "0" + hex val format
        # Data is decimal values

        lineAsList = [entry for entry in log_message.split() if entry]
        decimal_id = self.hexToDec(lineAsList[3])

        # The hex data from the message as a byte string
        data = self.convertToByteString(lineAsList[5:])

        # Get the ms offset of the message
        ms_offset = float(lineAsList[1])

        # Get the message number
        messageNum = int(lineAsList[0].rstrip(")"))

        # Decode the message
        decoded_message = self.dbc.decode_message(decimal_id, data)
        return ms_offset, self.jsonReadyID(decimal_id), decoded_message, messageNum


    """
    Given the hex data in the form of a list this method converts to a byte string

    PARAMS:
        dataAsList: The data as a list of hex values such as ['DE', 'AD', 'BE', 'EF']

    RETURNS: The data as a byte string such as b'\xde\xad\xbe\xef'
    """
    def convertToByteString(self, dataAsList):
        # Convert the hex values to bytes
        byte_string = bytes.fromhex("".join(dataAsList))

        # Add the "F" character at the beginning
        byte_string = b"" + byte_string
        return byte_string


    """
    Returns a map of all the signals to their length
    Used for checking if a signal is a status (1 or 2 bits) or a value
    Important because a status has an start_val, an exp_val, and an interval
    A value has a min, max, and an interval 

    RETURNS: dictionary of signal name (key) -> length of signal (value)
    """

    def getSignalToLengthDict(self):
        # Make the dictionary
        signalToLenDict = {}

        # Loop through all messages and signals and add to dict
        for message in self.dbc.messages:
            for signal in message.signals:
                signalToLenDict[signal.name] = signal.length

        return signalToLenDict


    """
    Converts a decimal ID to "0" + hex val format

    PARAMS:
        id: The decimal ID to convert
    
    RETURNS: The ID in "0" + hex val format
    """
    def jsonReadyID(self, id):
        return str(hex(id))


    """
    Converts a hex in the form "0" + hex val to decimal
    Also converts a hex in the form "0x" + hex val to decimal
    WILL NOT convert a "" + hex val to decimal

    PARAMS:
        hex: The hex value to convert

    RETURNS: The decimal value of the hex
    """
    def hexToDec(self, hex):
        if hex.startswith("0x"):
            return int(hex, 16)
        else:
            return int("0x" + hex.lstrip("0"), 16)
