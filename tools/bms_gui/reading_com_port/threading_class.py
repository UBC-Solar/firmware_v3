from PyQt5.QtCore import QThread,pyqtSignal
from serial import Serial
import writing_to_file.writing_logs as wt
from time import perf_counter

class reader_thread(QThread):
    """This is a threading class that will be used to read input from the serial port in the background.
    This process must be a thread, otherwise the gui does not respond until a new input is given.
    """
    new_information = pyqtSignal(str) #an unparsed with the new information will be transmitted as a signal
    voltage_values = []
    def __init__(self, port_name,MainWindow,update_limit = 0.1, file_path_to_record="",character_number=1,time_out=1):
        super().__init__()
        self.MainWindow=MainWindow
        self.UPDATE_LIMIT=update_limit#How many seconds to wait after each update

        self.file_path_to_record = file_path_to_record
        self.going_to_record=False
        self.port_name=port_name
        self.ser = Serial(port = self.port_name, baudrate=9600, timeout = time_out)
        self.current_port=port_name

        self.keep_reading = True
        self.character_number = character_number
        self.separator_character = "\n"

        self.voltage_graph=[]
        self.graph_voltage=False
    # def run(self):
    #     for x in range(10):
    #         #print(x)
    #         self.new_information.emit(x)
    #         sleep(1)



    def run(self,): #overriding built in run method and replacing it with our method. This is the functiont that the
        # thread will be running.
        potential_nexts={
            "s":["v","V"],
            "S": ["v", "V"],
            "v": ["t","T"],
            "V": ["t", "T"],
            "t": ["c","C"],
            "T": ["c", "C"],
            "c": ["Y","y"],
            "C": ["Y", "y"],
            "y": ["s","S"],
            "Y": ["s","S"]
        }
        self.ser.readline()
        current_string=""
        while self.keep_reading:
            #print("in loop")

            #print(self.ser.name, "is name")
            #print(self.ser.isOpen())

            start=perf_counter()
            alpha = self.ser.read()


            alpha=alpha.decode()
            current_string += alpha


            #print("in loop2")
            while alpha!=self.separator_character and self.keep_reading:
                #print("in nested loop")
                #this while loop will keep reading until the separator character is sent to signify that the data
                # transmission is over for current stringt
                alpha = self.ser.read().decode()
                current_string += alpha


            if len(current_string)>0:
                self.new_information.emit(current_string)
                if self.going_to_record:
                    #print(self.file_path_to_record)
                    wt.write_log(text=current_string, path_name = self.file_path_to_record)

            last_letter = current_string[0]
            string_to_change=False
            while self.keep_reading and perf_counter()-start<self.UPDATE_LIMIT or alpha not in potential_nexts[\
                    last_letter]:
                try:
                    alpha = self.ser.read().decode()
                except: break
                string_to_change=True
                #keep reading, but don't start recording until at least 1 minute passes

            if string_to_change:
                current_string=alpha
            else:
                current_string=""

        #print("outofloop")
