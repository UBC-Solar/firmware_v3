import os

"""
Returns all the lines to check assuming they are 'Rx'

PARAMS:
    log_file: The log file to read from
    type:     The type of lines to get. "Rx" is the recieved messages, 
                                        "Tx" is the transmitted messages, 
                                        "All" is BOTH recieved and transmitted messages

RETURNS: list of all the lines as a string to check
"""
def getValidLines(log_file, type):
    valid_lines = []

    # Get all the recieved and transmitted messages to check
    if type == "All":
        for line in log_file:
            if "Rx" in line or "Tx" in line and ")" in line:
                valid_lines.append(line)
    else:
        for line in log_file:
            if type in line:
                valid_lines.append(line)

    return valid_lines


"""
Gets the DBC path depending on the user's working directory

RETURNS: string of the DBC file path relative to the user
"""
def getDBCFile():
    return find_file("brightside_2.dbc", os.getcwd())


"""
Locates the path of a given file name

PARAMS:
    filename:       string of the file name to be found
    search_path:    string of the path to search for the file

RETURNS: string of the file path for a given file anme
"""
def find_file(filename, search_path):
    for root, dirs, files in os.walk(search_path):
        if filename in files:
            return os.path.join(root, filename)
    return None
