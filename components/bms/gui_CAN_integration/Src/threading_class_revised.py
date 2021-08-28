from PyQt5.QtCore import QThread, pyqtSignal
from serial import Serial
import writing_to_file.writing_logs as wt
from time import perf_counter


class reader_thread(QThread):
    """This is a threading class that will be used to read input from the serial port in the background.
    This process must be a thread, otherwise the gui does not respond until a new input is given.
    """
    new_information = pyqtSignal(
        str)  # an unparsed with the new information will be transmitted as a signal
    # a Qt signal is emitted when a particular event occurs. This signal is connected to a slot (a function), so that when signal emitted, slot is called in response
    voltage_values = []

    def __init__(self, port_name, MainWindow, update_limit=0.1, file_path_to_record="", character_number=1, time_out=1):
        super().__init__()
        self.MainWindow = MainWindow
        self.UPDATE_LIMIT = update_limit  # How many seconds to wait after each update

        self.file_path_to_record = file_path_to_record
        self.going_to_record = False
        self.port_name = port_name
        # baud rate is data transfer speed - serial port capable of transferring 9600 bits / second
        self.ser = Serial(port=self.port_name, baudrate=9600, timeout=time_out)
        self.current_port = port_name

        self.keep_reading = True
        self.character_number = character_number
        self.separator_character = "\n"

        self.voltage_graph = []
        self.graph_voltage = False

    # def run(self):
    #     for x in range(10):
    #         #print(x)
    #         self.new_information.emit(x)
    #         sleep(1)

    def run(self,):  # overriding built in run method and replacing it with our method. This is the function that the
        # thread will be running.
        potential_nexts = {
            "s": ["v", "V"],
            "S": ["v", "V"],
            "v": ["t", "T"],
            "V": ["t", "T"],
            "t": ["c", "C"],
            "T": ["c", "C"],
            "c": ["Y", "y"],
            "C": ["Y", "y"],
            "y": ["s", "S"],
            "Y": ["s", "S"]
        }
        self.ser.readline()
        current_string = ""
        while self.keep_reading:
            #print("in loop")

            #print(self.ser.name, "is name")
            # print(self.ser.isOpen())

            start = perf_counter()
            # default reads one byte, returns this byte of data, stores in variable alpha
            alpha = self.ser.read()

            alpha = alpha.decode()  # decoding bytes read into a string
            current_string += alpha + ":" + " "

            #print("in loop2")
            while self.keep_reading:

                #print("in nested loop")
                # this while loop will keep reading until the separator character is sent to signify that the data
                # transmission is over for current string

                if (alpha == "v" or alpha == "s" or alpha == "y"):

                    while alpha != self.separator_character:

                        try:
                            first_byte = self.ser.read().decode()  # serial data formatted as a string
                            alpha = self.ser.read().decode()
                            if (alpha == "\n" or first_byte == "\n"):
                                break
                            MSBs_bin = int(bin(int(first_byte)), base=2)
                            LSBs_bin = int(bin(int(alpha)), base=2)

                            # effectivly concatenating msbs and lsbs into 16 bit voltage integer expected
                            module_data = (MSBs_bin << 8 | LSBs_bin)

                            current_string += module_data + " "

                        except Exception as e:
                            # print(e)
                            pass

                elif (alpha == "t" or alpha == "c"):

                    while alpha != self.separator_character:

                        try:
                            alpha = self.ser.read().decode()
                            if alpha == "\n":
                                break
                            module_data = int(alpha)  # ensure casting works
                            current_string += module_data + " "

                        except Exception as e:
                            # print(e)
                            pass

            # separator character sent, out of loop

            if len(current_string) > 0:
                # emits a signal containing current_string
                self.new_information.emit(current_string)
                if self.going_to_record:
                    # print(self.file_path_to_record)
                    wt.write_log(text=current_string,
                                 path_name=self.file_path_to_record)

            last_letter = current_string[0]
            string_to_change = False

            while (self.keep_reading and perf_counter()-start < self.UPDATE_LIMIT) or (alpha not in potential_nexts[last_letter]):
                try:
                    alpha = self.ser.read().decode()
                except:
                    break
                string_to_change = True
                # keep reading, but don't start recording until at least 1 minute passes

            if string_to_change:
                current_string = alpha
            else:
                current_string = ""

        # print("outofloop")
