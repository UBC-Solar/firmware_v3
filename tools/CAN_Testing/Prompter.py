import os


"""
This class is used to separate the logic to obtain file names from classes involved in calculations
This allows other classes to re-use prompts when certain file names need to be used again
"""
class Prompter:
    def __init__(self):
        self.madeDecision = False


    """
    Handle which mode the script is to be used in
    Will prompt the user for the mode

    RETURNS: mode as a string and boolean decision to display passed tests
    """
    def chooseMode(self):
        # Choose stop mode only if at least 1 decision has been made
        if not self.madeDecision:
            mode = input("Enter mode (transmit or receive) as 't' or 'r': ")
        else:
            mode = input(
                "Enter mode ('transmit', 'receive', or 'stop') as 't', 'r', or 's': "
            )
        

        # Return what user wants
        if mode.lower() == "transmit":
            self.madeDecision = True
            return "Transmit", self.showPassedTestsPrompt()
        elif (
            mode.lower() == "receive"
            or mode.lower() == "r"
            or mode.lower() == "recieve"
        ):
            self.madeDecision = True
            return "Receive", self.showPassedTestsPrompt()
        elif mode.lower() == "stop" or mode.lower() == "s":
            return "Stop", False
        else:
            print("Invalid mode. Please enter either 'transmit' or 'receive'.")
            return "Error", False


    """
    Prompts the user for whether or not they want to display passed tests

    RETURNS: boolean decision to display passed tests
    """
    def showPassedTestsPrompt(self):
        # Do they want passed tests to show?
        displayPassed = input("Display passed tests? (y/n): ")
        if displayPassed.lower() == "y" or displayPassed.lower() == "yes":
            displayPassed = True
        else:
            displayPassed = False

        return displayPassed


    """
    Prompts the user for the Log file name and the JSON file name
    If the files cannot be opened the script ends

    RETURNS: tuple of two strings being log file path and json file path
    """
    def getFilesFromUser(self):
        # Try for log file
        log_name = input("Enter the name of the log file: ")
        search_path = os.getcwd()  # Get the current working directory
        log_file_path = self.find_file(log_name, search_path)
        try:
            open(log_file_path, "r")
        except:
            print("Error: Could not open log file. Check name and/or path")
            return
        
        # Try for JSON file
        json_name = input("Enter the name of the JSON file (Default is 'requirements.json'): ")
        if json_name == "":
            json_file_path = self.find_file("requirements.json", search_path)
        else:
            json_file_path = self.find_file(json_name, search_path)

        try:
            open(json_file_path, "r")
        except:
            print("Error: Could not open JSON file. Check name and/or path")
            return

        return log_file_path, json_file_path


    """
    Gets the DBC path depending on the user's working directory

    RETURNS: string of the DBC file path relative to the user
    """
    def getDBCFile(self):
        return self.find_file("brightside_2.dbc", os.getcwd())


    """
    Locates the path of a given file name

    PARAMS:
        filename:       string of the file name to be found
        search_path:    string of the path to search for the file

    RETURNS: string of the file path for a given file anme
    """
    def find_file(self, filename, search_path):
        for root, dirs, files in os.walk(search_path):
            if filename in files:
                return os.path.join(root, filename)
        return None
    

    """
    Prompts the user if they want to get the converted logs
    
    RETURNS: boolean decision to get converted logs
    """
    def getInfluxFormatLogs(self):
        # Prompt if user wants to get converted logs
        getLogs = input("Get converted logs? (y/n): ")

        # If yes, get the log file name
        if getLogs.lower() == "y" or getLogs.lower() == "yes":
            return True
        else:
            return False
