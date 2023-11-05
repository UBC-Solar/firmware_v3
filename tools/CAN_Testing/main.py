from Recieve import AutoChecker
from Prompter import Prompter
from Formatter import InfluxDBFormatter


# Instaniate prompter class for user inputs
prompt = Prompter()


""" BEGIN SCRIPT """
# Get the DBC file path
DBC_FILE_PATH = prompt.getDBCFile()

# Continuously get mode from user and perform actions
mode, displayPassed = prompt.chooseMode()
while mode != "Stop":
    if mode == "Transmit":
        pass
    elif mode == "Receive":
        # Instantiate new AutoChecker class and new Formatter class
        log_file, json_file = prompt.getFilesFromUser()
        receiver = AutoChecker(DBC_FILE_PATH, log_file, json_file, displayPassed)
        formatter = InfluxDBFormatter(log_file, json_file, displayPassed)

        # Check Log against requirements
        print("\n------------------ CHECKS ------------------\n")
        receiver.checkLog()

        # Write the log file to influx format if user wnats to
        if prompt.getInfluxFormatLogs():
            formatter.formatLogToInflux()

    # Keep re-prompting
    mode, displayPassed = prompt.chooseMode()
