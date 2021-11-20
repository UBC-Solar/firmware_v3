# Everything here is self-explanatory if you're able to run the program
# and see all the widgets, labels, bars, etc...

from PyQt5 import QtCore, QtGui, QtWidgets
from gui.terminal_class import terminal
from gui.box_class import box_info
from gui import graph_formatting
import serial.tools.list_ports  # to list port options
import os
from writing_to_file.writing_logs import generate_file_name
import pyqtgraph
from reading_com_port.parse_information import *
from reading_com_port.threading_class import reader_thread
from time import perf_counter

class Ui_MainWindow():
    # Converts a label to index. The voltage is always displayed on the first line, the temperature on the second
    # line, so on and so forth.
    letter_to_label_index = {"v": 0, "V": 0, "t": 1, "T": 1, "c": 2, "C": 2, "s": 3, "S": 3, 'y': None, 'Y': None}
    significant_digits = 3

    # A dictionary showing which bits correspond to which warnings:
    reason = {
        0: ["FAULT", "Communications Fault"],
        1: ["FAULT", "Self-test Fault"],
        2: ["FAULT", "Over-temperature Fault"],
        3: ["FAULT", "Under-voltage Fault"],
        4: ["FAULT", "Over-voltage Fault"],
        5: ["FAULT", "No voltage / Short Fault"],
        6: ["FAULT", "Temp out of expected range"],
        7: ["", ""],
        8: ["TRIP", "Balancing Active"],
        9: ["TRIP", "Low module voltage limit"],
        10: ["TRIP", "High module voltage limit"],
        11: ["TRIP", "Temp init for changing"],
        12: ["WARNING", "Low Voltage Warning"],
        13: ["WARNING", "High Voltage Warning"],
        14: ["WARNING", "Low Temperature Waring"],
        15: ["WARNING", "High Temperature Warning"],
        # 16: ["FAULT", "Over-current fault, source"],
        # 17: ["FAULT", "Over-current fault, load"]

    }

    def __init__(self, MainWindow):
        self.spacnow = 10 #spacing
        self.volt_num_of_entries = 0
        self.temp_num_of_entries = 0
        self.alpha = 0  # variable to keep track of which page we are on
        self.update_limit = 0.1 #time interval to read entries
        self.voltage_update_limit = self.update_limit
        self.total_voltage = 0  # variable to store what the sum of all voltages are



        self.minutes_time_interval = 1
        self.first_entry_time = None
        self.second_entry_time = None
        self.first_voltage_time = None
        self.second_voltage_time = None

        self._grapher_manager = graph_formatting.grapher_manager()
        self._temp_grapher_manager = graph_formatting.grapher_manager()
        self.going_to_record = False  # to see if we are saving the data sent
        self.boxes = []  # an array that stores objects of class box
        self.check_boxes = []  # stores QCheckbox objects
        self.check_box_labels = []  # stores QLabel objects for the checkboxes
        self.status_labels = []  # stores the status variables

        self.specific_name_of_text_file = "LOGS_" + generate_file_name() + ".csv"  # file name to save
        self.pack_wide_info = ""  # storing the Y variable. It is empty when initialized
        self.options=["1 minute", "2 minutes", "5 minutes", "15 minutes", "30 minutes", "1 hour", "2 hours", "3 hours",
         "5 hours",
         "10 hours"]

        self.minvolt = "None"
        self.maxvolt = "None"
        self.minvolt_module = "None"
        self.maxvolt_module = "None"

        self.mintemp = "None"
        self.maxtemp = "None"
        self.mintemp_module = "None"
        self.maxtemp_module = "None"

        self.event = "1 minute"

        # Generating the directory name:
        self.directory_to_save = str(os.getcwd())
        if self.directory_to_save[-3] + self.directory_to_save[-2] + self.directory_to_save[-1] == "GUI":
            self.directory_to_save = self.directory_to_save[0:-3]

        self.original_directory = str(self.directory_to_save)  # the initial directory that everything is set up in

        self.directory_to_save += r"writing_to_file\logs\ "[:-1]
        self.MainWindow = MainWindow  # main Widget

        # Insert the logo image icon
        path_begining = os.getcwd()
        if path_begining[-3:] != "GUI":
            path_begining += "\GUI"

        self.entry_limit = 100
        self.MainWindow.setWindowIcon(QtGui.QIcon(path_begining + "\logo_image\logo.png"))

        # THE PROGRAM AUTOMATICALLY DETECTS SCREEN SIZE:
        sizeObject = QtWidgets.QDesktopWidget().screenGeometry(-1)

        self.height = int(sizeObject.height())
        self.width = int(sizeObject.width())

        # self.width=1920/1.1
        # self.height=1080/1.1
        self.MainWindow.setGeometry(0, 0, self.width, self.height)

        # These values are used to scale the program and the graphics for different screens.
        self.original_height = 768
        self.original_width = 1366

        self.generate_things(MainWindow)  # Generates a bunch of widgets. The entire function is just setting up widget
        # variables. Nothing much to see here

        self.generate_terminal(self.MainWindow)  # Generates a terminal class. The terminal is a subclass of
        # Qscrollarea specifically tailored to act like terminal

        self.generate_row(first_or_sec=0)  # generates first row of box_info objects
        self.generate_row(y=self.rely(250), first_or_sec=16)  # generates second row of box_info objects

        self.generate_log_button(self.MainWindow)  # generates log button
        self.generate_path(self.MainWindow)  # Genereates path buttons and widgets in the bottom left corner
        self.generate_pack_wide_info(self.MainWindow)  # Generates the container for the Y variable
        self.generate_status_table()
        self.init_temp_graph()  # initializes the temperature graph
        self.init_voltage_graph()  # initalizes the voltage graph
        self.generate_tick_boxes()  # initializes the tick boxes for each module

        # We hide the voltage graph and temp graph since the program opens the main page initially
        self.voltage_graph.hide()
        self.temp_graph.hide()
        self.connect_buttons()  # connects buttons to functions

        self.highlight_curbut()

    
    

    def update_status_table(self, array, letter):
        """This function updates the status table, and the variables inside it"""

        if "s" not in letter and "S" not in letter:  # checks to see if the entry is for status'
            return None

        # Loops through all status and updates the table positions accordingly
        for i in range(len(array)):

            errors = str(array[i])  # a binary number stored as a string which is at most 16 digits

            if "\n" in errors:
                # If the error has \n in it, that means we have come to the end of the entry.
                # The function will not be able to parse this, so we move on
                continue

            # If the binary number has  zeros at the beginning, python will automatically delete them when decoding
            # message.
            # Here, we check if they are deleted. If so, we add them back:

            while len(errors) < 16:
                errors = "0" + errors

            for j in range(16):

                try:
                    current_letter = errors[j]  # get current bit
                except:

                    # This should never happen, since we make sure the lenght is at least 16 at the begining.
                    # I am keeping this except statement in just in case
                    current_letter = "0"

                if current_letter == "\n":
                    continue

                # We get the table item with coordiantes (i,j), and we update it. If the bit is 1, the color is
                # red, otherwise the color is white:
                if current_letter == "1":
                    current_color = QtGui.QColor(255, 0, 0)
                else:
                    current_color = QtGui.QColor(255, 255, 255)

                self.status_table.item(i, j).setBackground(current_color)

    def show_status_table(self):
        # Shows table
        self.status_table.show()
        self.MainWindow.update()

    def hide_status_table(self):
        self.status_table.hide()

    def display_status_page(self):
        if self.alpha != 3:
            self.hide_all()
            self.update_status_table([], "s")
            self.show_status_table()
        self.alpha = 3
        self.highlight_curbut()

    def table_is_clicked(self):
        # Called when the status table is clicked
        # This function evaluates the tile that has beeen clicked, and gives a popup message accordingly.

        # The coordinates of the cell clicked:
        r = self.status_table.currentRow()
        c = self.status_table.currentColumn()

        # If there is no problem with this tile, then we don't do anything.
        if self.status_table.item(r, c).background() != QtGui.QColor(255, 0, 0):
            return None

        # Creating an error message using the dictionary defined at the top:
        alpha = QtWidgets.QMessageBox(self.MainWindow)

        if r == 32:
            mnum = "Y CODE"
        else:
            mnum = "Module " + str(1 + r)
        alpha.setText(mnum + "\n" + self.reason[int(c)][0] + ": " + self.reason[int(c)][1])
        # alpha.setIcon()

        # Setting an icon dependin on the warning
        if self.reason[int(c)][0] == "WARNING":
            alpha.setIcon(QtWidgets.QMessageBox.Warning)
        elif self.reason[int(c)][0] == "FAULT":
            alpha.setIcon(QtWidgets.QMessageBox.Critical)
        else:
            alpha.setIcon(QtWidgets.QMessageBox.Information)

        # showing the pop up:
        alpha.show()

    def generate_status_table(self):

        # Initializing the status table:
        self.status_table = QtWidgets.QTableWidget(parent=self.MainWindow)
        self.status_table.setRowCount(33)  # 33 rows
        self.status_table.setGeometry(self.relx(10), self.rely(50), self.relx(1300),
                                      self.rely(650))  # relative coordinates
        self.status_table.setColumnCount(16)  # 16 columns

        # looping through every cell, and adding a table widget item to each cell:
        for i in range(33):
            for j in range(16):
                self.status_table.setItem(i, j, QtWidgets.QTableWidgetItem())

        # These will be displayed on the columns:
        head = ['COMM', 'ST', 'OT', 'UV', 'OV', 'NO_VOLT', 'TEMP_RANGE', '   ', 'BAL', 'LLIM',
                'HLIM', 'CHARGE_OT', 'LOW_V', 'HIGH_V', 'LOW_T', 'HIGH_T', 'OC_IN',
                'OC_OUT']

        # We set the columns width depending on how long the headline is:
        for i in range(16):
            self.status_table.setColumnWidth(i, len(head[i]) * 13)

        # Setting the heights for each row:
        for i in range(32):
            self.status_table.setRowHeight(i, 1)

        self.status_table.setHorizontalHeaderLabels(
            ['COMM', 'ST', 'OT', 'UV', 'OV', 'NO_VOLT', 'TEMP_RANGE', '', 'BAL', 'LLIM',
             'HLIM', 'CHARGE_OT', 'LOW_V', 'HIGH_V', 'LOW_T', 'HIGH_T', 'OC_IN',
             'OC_OUT'])

        self.status_table.clicked.connect(
            self.table_is_clicked)  # If the table is clicked, this function will be executed.

        self.status_table.hide()

    def generate_terminal(self, MainWindow):
        # Generates a terminal (the terminal class is in the file titled 'terminal_class'
        self.terminal = terminal(x=self.relx(500), y=self.rely(450), w=self.relx(800), h=self.rely(250),
                                 window=MainWindow)
        self.terminal_text = ""  # no text initially

    def add_to_terminal(self, line):
        # add a line to the terminal widget:

        # make sure the last two characters are \n\n:
        while line[-1] + line[-2] != "\n\n":
            line += "\n"

        self.terminal_text += line

        if len(self.terminal_text) > 2000:
            to_cut = len(self.terminal_text) - 2000
            self.terminal_text = self.terminal_text[to_cut:]

        # setting text:
        self.terminal.setText(self.terminal_text)

        # scrolling to the end:
        scroller = self.terminal.verticalScrollBar()
        scroller.setValue(scroller.maximum())

    def add_port_options(self):

        # This reads the ports that are on this computer, and adds all of them as options to the dropdown selection:
        ports = serial.tools.list_ports.comports()
        for p in ports:
            self.comboBox.addItem(p.name)

        # initalizing the port_name variable:
        self.port_name = self.comboBox.currentText()

    def change_font(self,_):
        if self.resizetextBox.currentText()=="Large":
            self.main_button.setFont(QtGui.QFont("Segoe UI", self.fonty(14))) 
            self.voltage.setFont(QtGui.QFont("Segoe UI", self.fonty(14)))
            self.temp.setFont(QtGui.QFont("Segoe UI", self.fonty(14)))
            self.status_button.setFont(QtGui.QFont("Segoe UI", self.fonty(14)))
            self.pack_wide_box.setFont(QtGui.QFont("Segoe UI", self.fonty(14)))
            self.pack_wide_voltage.change_textsize(self.pack_wide_voltage.labels,self.fonty(11)) #find the right size
            for i in range (32):   
                self.check_box_labels[i].setFont((QtGui.QFont("Segoe UI", self.fonty(10)))) #find the right size
                self.boxes[i].change_textsize(self.boxes[i].labels,self.fonty(11)) #find the right size
            self.status.setFont(QtGui.QFont("Segoe UI", self.fonty(10)))
            self.submit_path.setFont(QtGui.QFont("Segoe UI", self.fonty(13)))
            self.logbutton.setFont(QtGui.QFont("Helvetica", self.fonty(10)))

        elif self.resizetextBox.currentText()=="Medium":
            self.main_button.setFont(QtGui.QFont("Segoe UI", self.fonty(12))) 
            self.voltage.setFont(QtGui.QFont("Segoe UI", self.fonty(12)))
            self.temp.setFont(QtGui.QFont("Segoe UI", self.fonty(12))) 
            self.status_button.setFont(QtGui.QFont("Segoe UI", self.fonty(12)))
            self.pack_wide_box.setFont(QtGui.QFont("Segoe UI", self.fonty(12)))
            self.pack_wide_voltage.change_textsize(self.pack_wide_voltage.labels, self.fonty(7))
            for i in range (32):
                self.check_box_labels[i].setFont((QtGui.QFont("Segoe UI", self.fonty(7))))
                self.boxes[i].change_textsize(self.boxes[i].labels,self.fonty(7))
            self.status.setFont(QtGui.QFont("Segoe UI", self.fonty(8)))
            self.submit_path.setFont(QtGui.QFont("Segoe UI", self.fonty(10)))
            self.logbutton.setFont(QtGui.QFont("Helvetica", self.fonty(6)))

        else:
            self.main_button.setFont(QtGui.QFont("Segoe UI", self.fonty(8))) 
            self.voltage.setFont(QtGui.QFont("Segoe UI", self.fonty(8)))
            self.temp.setFont(QtGui.QFont("Segoe UI", self.fonty(8))) 
            self.status_button.setFont(QtGui.QFont("Segoe UI", self.fonty(8)))
            self.pack_wide_box.setFont(QtGui.QFont("Segoe UI", self.fonty(8)))
            self.pack_wide_voltage.change_textsize(self.pack_wide_voltage.labels,self.fonty(5))
            for i in range (32):
                self.check_box_labels[i].setFont((QtGui.QFont("Segoe UI", self.fonty(5))))
                self.boxes[i].change_textsize(self.boxes[i].labels,self.fonty(5))
            self.status.setFont(QtGui.QFont("Segoe UI", self.fonty(5)))
            self.submit_path.setFont(QtGui.QFont("Segoe UI", self.fonty(8)))
            self.logbutton.setFont(QtGui.QFont("Helvetica", self.fonty(5)))
            

    # Every coordinate for this GUI was originally designed to be on a 1366x768 screen.
    # In order to resize everything, we need to scale the original coordinate to your screensize
    def relx(self, number):
        # Relx scales an x coordinate depending on the curernt width of your screen.
        return (number / self.original_width) * self.width

    def rely(self, number):
        # rely scales a y coordinate depending on the curernt height of your screen.
        return (number / self.original_height) * self.height

    def fonty(self, number):
        # Scales the fonts.
        # To scale the fonts, we look at both scaling depending on the height and width of the screen.
        # To make sure everything fits, we take whicever gives a smaller size.
        a = self.rely(number)
        aa = self.relx(number)
        #print(self.width)
        #print(self.height)
        if a < aa:
            final = a
        else:
            final = aa

        return final

    def generate_things(self, MainWindow):
        # THIS FUNCTION GENERATES LOTS OF BUTTONS, LABELS AND WIDGETS THAT ARE USED IN THE PROGRAM.
        # The entire function is just setting up variables. There isn't really anything to see.
        # If you want to see a bunch of settings get configured, go ahead and read the function
        # all variables are set with relative positions (using the self.rely and self.relx methods)

        self.MainWindow.setObjectName("self.MainWindow")
        # self.MainWindow.resize(1200, 650)

        self.centralwidget = QtWidgets.QWidget(self.MainWindow)
        self.centralwidget.setObjectName("centralwidget")

        self.resizetextBox = QtWidgets.QComboBox(self.centralwidget)
        self.resizetextBox.setGeometry(QtCore.QRect(self.relx(20), self.rely(450), self.relx(100), self.rely(40)))
        self.resizetextBox.setObjectName("resizetextBox")
        self.resizetextBox.addItems(["Large","Medium","Small"])
        self.resizetextBox.activated.connect(self.change_font)

        self.main_button = QtWidgets.QPushButton(self.centralwidget)
        self.main_button.setGeometry(QtCore.QRect(self.relx(20), self.rely(0), self.relx(141), self.rely(51)))
        self.main_button.setObjectName("main")
        self.main_button.setFont(QtGui.QFont("Segoe UI", self.fonty(14))) 

        self.voltage = QtWidgets.QPushButton(self.centralwidget)
        self.voltage.setGeometry(QtCore.QRect(self.relx(160), self.rely(0), self.relx(141), self.rely(51)))
        self.voltage.setObjectName("voltage")
        self.voltage.setFont(QtGui.QFont("Segoe UI", self.fonty(14)))

        self.temp = QtWidgets.QPushButton(self.centralwidget)
        self.temp.setGeometry(QtCore.QRect(self.relx(300), self.rely(0), self.relx(141), self.rely(51)))
        self.temp.setObjectName("temp")
        self.temp.setFont(QtGui.QFont("Segoe UI", self.fonty(14))) 

        self.status_button = QtWidgets.QPushButton(self.centralwidget)
        self.status_button.setGeometry(QtCore.QRect(self.relx(440), 0, self.relx(141), self.rely(51)))
        self.status_button.setObjectName("status")
        self.status_button.setFont(QtGui.QFont("Segoe UI", self.fonty(14)))

        self.comboBox = QtWidgets.QComboBox(self.centralwidget)
        self.comboBox.setGeometry(QtCore.QRect(self.relx(600), self.rely(10), self.relx(181), self.rely(31)))
        self.comboBox.setObjectName("comboBox")
        self.add_port_options()

        self.minutes_to_see = QtWidgets.QComboBox(self.centralwidget)
        self.minutes_to_see.setGeometry(QtCore.QRect(self.relx(600), self.rely(400), self.relx(181), self.rely(40)))




        self.minutes_to_see.hide()
        options = self.options
        for o in options:
            self.minutes_to_see.addItem(o)



        self.MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QtWidgets.QMenuBar(self.MainWindow)
        self.menubar.setGeometry(QtCore.QRect(0, 0, self.relx(827), self.rely(21)))
        self.menubar.setObjectName("menubar")

        self.MainWindow.setMenuBar(self.menubar)
        self.statusbar = QtWidgets.QStatusBar(self.MainWindow)
        self.statusbar.setObjectName("statusbar")

        self.MainWindow.setStatusBar(self.statusbar)

        self.retranslateUi()
        QtCore.QMetaObject.connectSlotsByName(self.MainWindow)

        self.start_reading = QtWidgets.QPushButton(MainWindow)
        self.start_reading.setGeometry(QtCore.QRect(self.relx(800), self.rely(10), self.relx(181), self.rely(31)))
        self.start_reading.setObjectName("startbutton")
        self.start_reading.setText("Start Reading Port")
        self.start_reading.setFont(QtGui.QFont("Segoe UI", self.fonty(9)))  #14
        # self.stop_reading = QtWidgets.QPushButton(MainWindow)
        # self.stop_reading.setGeometry(QtCore.QRect(1000, 10, 181, 31))
        # self.stop_reading.setObjectName("stopbutton")
        # self.stop_reading.setText("Stop Reading Port")

        self.status = QtWidgets.QLabel(MainWindow)
        self.status.setGeometry(self.relx(1000), self.rely(10), self.relx(300), self.rely(31))
        self.status.setText("Not reading any port at the moment.")
        self.status.setFont(QtGui.QFont("Segoe UI", self.fonty(10)))
    

    def retranslateUi(self):
        # CONFIGURING SOME NAMES:
        _translate = QtCore.QCoreApplication.translate
        self.MainWindow.setWindowTitle(_translate("self.MainWindow", "Battery Management"))
        self.main_button.setText(_translate("self.MainWindow", "Main Tab"))
        self.voltage.setText(_translate("self.MainWindow", "Voltage"))
        self.temp.setText(_translate("self.MainWindow", "Temperature"))
        self.status_button.setText(_translate("self.MainWindow", "Status"))

    def change_path(self):
        # CHANGING THE PATH TO SAVE THE LOG FILE (USING FILE NAVIGATOR) :
        directory_to_save = QtWidgets.QFileDialog.getExistingDirectory(self.MainWindow, 'Select Folder')

        if len(directory_to_save) > 1:
            # Check if the user clicked on a file
            self.directory_to_save = directory_to_save + \
                                     "/" + self.specific_name_of_text_file

        # Set it as text so the user knows that the pass has changed
        self.path_widget.setText(self.directory_to_save)
        try:
            # If the worker thread has been initialized, tell the worker about the new file path.
            # If the worker does not exist yet, do nothing
            self.worker.file_path_to_record = self.directory_to_save



        except:
            pass

    def update_time_interval(self,event):

        #translation = {"1 minute", "15 minutes", "30 minutes", "1 hour", "2 hours", "3 hours", "5 hours", "10 hours"}
        num=parse_float_from_str(string = event)
        self.minutes_time_interval = num
        if "hour" in event:
            self.minutes_time_interval*=60

        self.minutes_time_interval = int(self.minutes_time_interval)
############################################################
        # if "10 hour" in event:
        #     self.worker.UPDATE_LIMIT=1/4
        #     print("it worked")

        self.event = event
        print(self.minutes_time_interval)

        self.decide_on_graph()
    def connect_buttons(self):
        # This function connects buttons to functions

        self.voltage.clicked.connect(self.display_voltage_page)  # Displays voltage page
        self.start_reading.clicked.connect(self.thread)  # starts reading com port
        self.logbutton.clicked.connect(self.start_recording)  # starts recording logs
        self.main_button.clicked.connect(self.display_main)  # displays main page
        self.temp.clicked.connect(self.display_temp_page)  # displays temperature page
        self.status_button.clicked.connect(self.display_status_page)  # displays status table
        self.submit_path.clicked.connect(self.change_path)  # changes the path
        self.minutes_to_see.currentTextChanged.connect(self.update_time_interval)
    def highlight_curbut(self):
        buttons = [self.main_button, self.voltage, self.temp, self.status_button]

        for i in range(len(buttons)):
            if i == self.alpha:
                color = "lightgreen"
            else:
                color = "lightgray"

            buttons[i].setStyleSheet(f"background-color: {color};")

    def decide_on_graph(self):
        # Decide on which graph you want to render right now.

        if self.alpha == 1:
            # we are on voltage page
            self.draw_voltage_graph()
        elif self.alpha == 2:
            # we are on temperature page
            self.draw_temp_graph()


    def checkbox_style(self,checkbox_number):
        if checkbox_number>=len(self.boxes):
            checkbox_number=len(self.boxes)-1


        color = self.boxes[checkbox_number].color
        add_some_style_boi = """
                QCheckBox::indicator{
                    width:100%;
                    height:100%;
                    background-color:"""+color+""";}
                    QCheckBox::indicator:unchecked
                    {
                    background-color : white;
                    }
                """

        return add_some_style_boi
    def generate_tick_boxes(self):
        # Generates the tick boxes used to define which modules you want to graph:

        spacing = 23  # spacing can be an absoloute value, since everything will be converted to a relative form when
        # actually setting the geometry

        # GENERATE THE 16 CHECKBOXES ON THE LEFT:
        for i in range(16):
            add_some_style_boi = self.checkbox_style(i)
            # CONFIGURE CHECK BOX:
            tickbox = QtWidgets.QCheckBox(parent=self.MainWindow, )
            tickbox.setGeometry(self.relx(1000), self.rely(70 + i * spacing), self.relx(30), self.rely(20))
            tickbox.setStyleSheet(
                add_some_style_boi
            )

            tickbox.stateChanged.connect(self.decide_on_graph)
            tickbox.hide()

            # store checkbox:
            self.check_boxes.append(tickbox)

            # CONFIGURE THE LABEL FOR THE CHECKBOX:
            label = QtWidgets.QLabel(parent=self.MainWindow)
            label.setAlignment(QtCore.Qt.AlignLeft)
            label.setGeometry(self.relx(840), self.rely(70 + i * spacing), self.relx(145), self.rely(15))
            label.setFont(QtGui.QFont("Segoe UI", self.fonty(10))) 
            label.setText("Module " + str(i + 1))
            label.hide()

            # STORE LABEl:
            self.check_box_labels.append(label)

        # GENERATE THE RIGHT 16 CHECKBOXES WITH DIFFERENT AND LABELS:
        for i in range(16):
            add_some_style_boi = self.checkbox_style(i)

            tickbox = QtWidgets.QCheckBox(parent=self.MainWindow)
            tickbox.setGeometry(self.relx(1250), self.rely((70) + i * spacing), self.relx(30), self.rely(20))
            # tickbox.setChecked(True)
            tickbox.stateChanged.connect(self.decide_on_graph)
            tickbox.setStyleSheet(
                add_some_style_boi
            )

            tickbox.hide()

            # tickbox.hide()
            label = QtWidgets.QLabel(parent=self.MainWindow)
            label.setAlignment(QtCore.Qt.AlignLeft)

            label.setGeometry(self.relx(1070), self.rely(70 + i * spacing), self.relx(145), self.rely(15))
            label.setText("Module " + str(i + 17))
            label.setFont(QtGui.QFont("Segoe UI", self.fonty(10)))

            label.hide()
            self.check_box_labels.append(label)
            self.check_boxes.append(tickbox)

    def hide_tickboxes(self):
        # hide every checkbox and the corresponding label (used when we are going to a page where we should not display
        # the checkboxes)
        for k in range(32):
            self.check_boxes[k].hide()
            self.check_box_labels[k].hide()

    def show_tickboxes(self):
        # Show every checkbox and it's label
        for k in range(32):
            self.check_boxes[k].show()
            self.check_box_labels[k].show()

    def display_main(self):
        # display main page:
        if self.alpha != 0:
            # We are not on the main page, we need to render the main page


            self.hide_all()  # hides everyhting
            self.render_main()  # shows widgets in main
            self.hide_tickboxes()

            if self.alpha == 1:
                # if we were on the voltage page previuosly, we hide the voltage graph
                self.voltage_graph.hide()

            elif self.alpha == 2:
                # if we were on the temperature page previuosly, we hide the voltage graph

                self.temp_graph.hide()

        # We are now on the main page. We will store this fact in the self.alpha variable
        self.alpha = 0
        self.highlight_curbut()

    def generate_box(self, x, y, xx, yy, module):
        # generates  a box_info class and stores the object
        box = box_info(self.centralwidget, x, y, xx, yy, module, self.height, self.width)
        box.change_voltage(0)
        self.boxes.append(box)

        return box

    def generate_row(self, first_or_sec, x=20, y=60, num=16, spacing=84):

        # Generates a row of relatively spaced out box_info class objects

        # relatively positioning things:
        if x == 20:
            x = self.relx(x)
        if y == 60:
            y = self.rely(y)
        if spacing == 84:
            spacing = self.relx(spacing)
        width = self.relx(40)
        height = self.rely(80)

        # Looping through row and generating first 16 lines of modules:
        for i in range(num):
            self.generate_box(x + spacing * i, y, width, height, i + first_or_sec)  # generating progress bar

    def generate_log_button(self, MainWindow):
        # CONFIGURING LOG BUTTON:

        self.logbutton = QtWidgets.QPushButton(MainWindow)
        self.logbutton.setGeometry(QtCore.QRect(self.relx(290), self.rely(620), self.relx(200), self.rely(51)))
        self.logbutton.setObjectName("logbutton")
        self.logbutton.setText("Press To Start Recording Logs")
        self.logbutton.setFont(QtGui.QFont("Helvetica", self.fonty(10)))

    def start_recording(self):
        # Gets called when the logbutton is pressed.

        # Change the variable to the opposite of what it initally was:
        if self.going_to_record == True:
            self.logbutton.setText("Press To Start Recording Logs")
            bool_to = False

        else:
            self.logbutton.setText("Recording. Press to stop.")
            bool_to = True

        try:
            # If the worker thread is listening to the port, tell it about the update
            self.worker.going_to_record = bool_to

        except:
            pass

        # update the actual variable as well
        self.going_to_record = bool_to

    def generate_path(self, MainWindow):
        # generates all widgets related to storing the path of the log directory:

        # configure default path to save:
        self.directory_to_save = str(self.original_directory)
        if self.directory_to_save[-3] + self.directory_to_save[-2] + self.directory_to_save[-1] == "GUI":
            self.directory_to_save = self.directory_to_save[0:-3]

        if self.directory_to_save[-1] != "\ "[:-1]:
            self.directory_to_save += "\ "[:-1]

        self.directory_to_save += r"writing_to_file\logs\ "[:-1]

        # configure the widget where we display the path to store logs
        self.path_widget = QtWidgets.QLineEdit(MainWindow)
        self.path_widget.setGeometry(self.relx(5), self.rely(675), self.relx(480), self.rely(25))
        self.path_widget.setText(self.directory_to_save)
        self.path_widget.setFont(QtGui.QFont("Segoe UI", self.fonty(10)))

        # configure the label that says "Directory to Save: (absoloute)"
        self.path_label = QtWidgets.QLabel(MainWindow)
        self.path_label.setGeometry(self.relx(5), self.rely(625), self.relx(250), self.rely(51))
        self.path_label.setText("Directory to Save:")
        self.path_label.setFont(QtGui.QFont("Segoe UI", int(self.rely(10) * 1.2)))
        style = """font-weight:bold"""
        self.path_label.setStyleSheet(style)

        # Configure the button used to change path (this button opens the file system when clicked):
        self.submit_path = QtWidgets.QPushButton(self.centralwidget)
        self.submit_path.setGeometry(QtCore.QRect(self.relx(5), self.rely(580), self.relx(120), self.rely(51)))
        self.submit_path.setObjectName("submit_path ")
        self.submit_path.setText("Change Path")
        self.submit_path.setFont(QtGui.QFont("Segoe UI", self.fonty(13)))

    def generate_pack_wide_info(self, MainWindow, textforthis=None):
        # Generate a box_info object to store pack wide information
        if textforthis is None:
            textforthis = 'Y:CODE'

        # configure the label that stores the information for the "Y" variable
        self.pack_wide_box = QtWidgets.QLabel(parent=MainWindow)
        self.pack_wide_box.setFont(QtGui.QFont("Segoe UI", self.fonty(14)))

        self.pack_wide_box.setText(textforthis)
        self.pack_wide_box.setGeometry(self.relx(5), self.rely(520), self.relx(300), self.rely(51))
        self.pack_wide_box.show()

        # Configure box_info object that stores the sum of all voltages:

        self.pack_wide_voltage = box_info(self.MainWindow,
                                          self.relx(250),
                                          self.rely(445),
                                          self.relx(200),
                                          self.rely(51),
                                          0,
                                          self.height,
                                          self.width,
                                          text_list=[self.spacnow*" "+"Pack Wide Voltage: " + str(self.total_voltage),
                                                     self.spacnow*" "+"Min V: "+str(self.minvolt) + " (Module: None)",
                                                     self.spacnow*" "+"Max V: "+str(self.maxvolt)+ " (Module: None)",
                                                     self.spacnow*" "+"Min T: " + str(self.mintemp) + " (Module: None)",
                                                     self.spacnow*" "+"Max T: " + str(self.maxtemp) + " (Module: None)"],
                                          spacing = 20
                                          )

        for l in self.pack_wide_voltage.labels:
            origi = l.geometry()
            l.setAlignment(QtCore.Qt.AlignLeft)
            l.setGeometry(origi.x(),origi.y(),origi.width(),self.rely(50))
            # l.setStyleSheet("background:gray")

            # l.setStyleSheet("background:gray")


            # l.setGeometry(                                          self.relx(250),
            #                               self.rely(445),
            #                               self.relx(200),
            #                               self.rely(51),)


        self.pack_wide_voltage.voltage_bar.setAlignment(QtCore.Qt.AlignLeft)

        self.pack_wide_voltage.labels[0].setFixedHeight(self.rely(22))
        self.pack_wide_voltage.voltage_bar.setOrientation(QtCore.Qt.Horizontal)

    def hide_all(self, ):
        # Hide everything on page, unless the main
        for a in self.boxes:
            a.hide_all()  # this is the hide_all method of the box_info class. Not to be confused with the hide_all
            # method of the current class we are inside.

        try:
            # If voltage has been initalized (it has)
            self.voltage_graph.hide()
        except:
            pass  # variable has not been initialized yet (this should never happen, adding the except statement just in
            # e)
        try:

            self.temp_graph.hide()
        except:
            pass  # variable has not been initialized yet

        try:
            self.hide_tickboxes()
        except:
            pass

        self.hide_status_table()
        self.minutes_to_see.hide()
    def render_main(self):
        # Show all widgets that is meant to display on the main page
        for a in self.boxes:
            a.show_all()

        # self.terminal.show()
        try:
            self.pack_wide_box.show_all()
        except:
            pass
        self.path_widget.show()
        self.logbutton.show()
        self.path_label.show()
        self.submit_path.show()

    def display_temp_page(self):
        # displays temperature page
        if self.alpha != 2:
            # If we are not already on the page:
            # hide everything,draw graph,show the graph, show the tickboxes, and tell the main window to update
            self.hide_all()
            self.draw_temp_graph(first_time = True)
            self.temp_graph.show()
            self.show_tickboxes()
            self.minutes_to_see.show()
            self.MainWindow.update()

        # We store the fact that we are in the self.alpha variable:
        self.alpha = 2
        self.highlight_curbut()

    def init_temp_graph(self):
        # initalize the temperature graph:

        xs = [i for i in range(1, 33)]  # initally, the points on the x axis are arbitrary
        thing = xs  # this is also arbitrary since there is no data to plot yet

        # Configuring the PlotWidget from the library called pyqtgraph
        self.temp_graph = pyqtgraph.PlotWidget(parent=self.MainWindow, x=xs, y=thing)
        self.temp_graph.setGeometry(self.relx(10), self.rely(55), self.relx(800), self.rely(340))
        self.temp_graph.setBackground('w')
        # self.temp_graph.addLegend()  # adding legend

        # setting the names for the axes:
        self.temp_graph.getPlotItem().setLabel("left", text="Temperature", units="\N{DEGREE SIGN}C")
        self.temp_graph.getPlotItem().setLabel("bottom", text="Time", units="s")

    def draw_voltage_graph(self,first_time = False):


        self.voltage_graph.clear()

        for i in range(32):

            if self.check_boxes[i].isChecked():


                y_field, xs = self._grapher_manager.get_state(i,self.event)

                item = pyqtgraph.PlotCurveItem(x=xs, y=y_field, pen=pyqtgraph.mkPen(self.boxes[i].color,
                                                                                    width=self.rely(3)),)
                                               # name="Module " + str(i + 1) +
                                               #      "\nMin:" + minstring + " " + "Max: " +
                                               #      maxstring

                self.voltage_graph.addItem(item)

                self.voltage_graph.show()

    def draw_temp_graph(self,first_time = False):

        self.temp_graph.clear()
        for i in range(32):

            if self.check_boxes[i].isChecked():
                # print(i)
                cur_event = self.event

                if first_time:
                    # print("first")
                    cur_event += " d"

                y_field, xs = self._temp_grapher_manager.get_state(i, self.event)

                item = pyqtgraph.PlotCurveItem(x=xs, y=y_field, pen=pyqtgraph.mkPen(self.boxes[i].color,
                                                                                    width=self.rely(3)),
                                               )
                # name = "Module " + str(i + 1) +
                # "\nMin:" + minstring + " " + "Max: " +
                # maxstring
                # invisible_item = pyqtgraph.PlotCurveItem(x=newxs, y=newy_field, pen=pyqtgraph.mkPen((255, 255, 255)))
                # self.temp_graph.addItem(invisible_item)

                self.temp_graph.addItem(item)

                self.temp_graph.show()

    def display_voltage_page(self):

        if self.alpha != 1:
            self.hide_all()
            self.draw_voltage_graph(first_time = True)
            self.voltage_graph.show()
            self.show_tickboxes()

            self.minutes_to_see.show()
            self.MainWindow.update()
        self.alpha = 1
        self.highlight_curbut()

    # def update_voltages(self):
    #     self.voltages = []
    #     for k in self.boxes:
    #         self.voltages.append(parse_float_from_str(str(k.labels[0].text())))

    # def update_temps(self):
    #     self.temps = []
    #     for k in self.boxes:
    #         self.temps.append(parse_float_from_str(str(k.labels[1].text())))

    def init_voltage_graph(self):
        # these two are arbitrary since there is no data to plot yet
        xs = [i for i in range(1, 33)]
        thing = xs

        # configuring the PlotWidget:
        self.voltage_graph = pyqtgraph.PlotWidget(parent=self.MainWindow, x=xs, y=thing, name="alpha")
        # self.voltage_graph.addLegend()  # added legend
        self.voltage_graph.setGeometry(self.relx(10), self.rely(55), self.relx(800), self.rely(340))
        self.voltage_graph.setBackground('w')  # set background

        # labeled axes:
        self.voltage_graph.getPlotItem().setLabel("left", text="Voltage", units="V")
        self.voltage_graph.getPlotItem().setLabel("bottom", text="Time", units="s")

    def update_values(self, array_of_values, label_index, letter="V: "):
        # Gets called when the worker thread reads a new values from the serial port

        units = {0: "V", 1: "\N{DEGREE SIGN}C", 2: '%'}

        # if there is a unit for this index, use it:
        if label_index in units:
            u = units[label_index]
        else:
            u = ""

        # Looping through every box_info object and updating their information:

        for i in range(len(self.boxes)):
            try:
                # if box exists, update the text in it:
                self.boxes[i].labels[label_index].setText(
                    letter + str(signi_digs(round(array_of_values[i], self.significant_digits),self.significant_digits)) + " " + u)

            except:
                pass

            if label_index == 0:
                # The input is voltages:
                try:
                    # call the change voltage method of the box_info object:
                    self.boxes[i].change_voltage(value=array_of_values[i])
                except:
                    break

                # store this voltage in the file of previous voltages:
                try:
                    self.boxes[i].update_past_voltages(array_of_values[i])
                except:
                    pass

            elif label_index == 1:
                # the input is temperature, we call the box_info method to store this temperature:
                try:
                    self.boxes[i].update_past_temps(array_of_values[i])
                except:
                    pass

            elif label_index == 3:
                # This is a status input, we will see if he "BAL" of the status is 1 or 0:
                try:
                    if '1' not in str(array_of_values[i][8]):

                        color = "transparent"
                    else:
                        color = "lightblue"
                except:
                    color = "transparent"

                # setting the background color:
                stylesheet = f"background-color:{color};"

                self.boxes[i].labels[label_index].setStyleSheet(stylesheet)
                self.boxes[i].labels[label_index].setText('BAL')

        self.MainWindow.update()

    def thread(self):

        if self.port_name == "":
            return None

        try:
            type(self.worker)
            yes = False
        except:
            yes = True

        if yes:
            # never been initialized before, initialize worker
            self.port_name = self.comboBox.currentText()

            # This is a subclass of the QThread class.
            self.worker = reader_thread(self.port_name, MainWindow=self.MainWindow,
                                        file_path_to_record=self.directory_to_save +
                                                            self.specific_name_of_text_file,
                                        update_limit = self.update_limit)

            self.worker.going_to_record = True

            self.worker.new_information.connect(self.thread_dynamic_update)

            self.status.setText("Reading port: " + self.port_name)

            self.worker.going_to_record = self.going_to_record

            self.worker.start()
            self.start_reading.setText("Stop Reading")

        elif self.port_name != self.comboBox.currentText():
            try:
                self.start_reading.setText("Stop Reading")

                self.port_name = self.comboBox.currentText()
                # exiting previous port:
                self.worker.keep_reading = False  # we close this so the worker can exit the loop
                self.worker.ser.close()

                self.worker.keep_reading = True

                self.worker.ser = serial.Serial(port=self.port_name, baudrate=9600, timeout=1)

                self.status.setText("Reading port: " + self.port_name)
                self.worker.start()

            except Exception as e:
                self.start_reading.setText("Start Reading")
                self.status.setText("Could not open port: " + self.port_name)


        else:
            popup = QtWidgets.QMessageBox.question(self.MainWindow, "", "Are you sure you want to stop reading port?",
                                                   QtWidgets.QMessageBox.No | QtWidgets.QMessageBox.Yes, )

            if popup == QtWidgets.QMessageBox.Yes:
                self.port_name = "Not reading any port at the moment."  # changint this so the program knows we are not on
                # the same port anymore

                self.worker.keep_reading = False  # we close this so the worker can exit the loop
                self.worker.ser.close()

                self.status.setText("Not reading any port at the moment.")

                self.start_reading.setText("Start Reading")


            else:
                pass

    # def stop_recording(self):
    #     try:
    #         self.worker.keep_reading=False
    #         self.worker.close()
    #     except:
    #         pass

    @staticmethod
    def get_slopes(arry, x_interval, how_many=5):

        if len(arry)<how_many:
            return 0
        total = 0

        for i in range(1, 1 + how_many):
            i = len(arry) - 1 - i
            if i < -how_many:
                break
            delta_y = arry[i + 1] - arry[i]
            delta_x = x_interval
            total += delta_y / delta_x
            # print(arry[i], arry[i + 1], delta_y / delta_x)

        return total / how_many


    def thread_dynamic_update(self, value):
        if self.first_entry_time is None:
            self.first_entry_time = perf_counter()
        elif self.second_entry_time is None:
            self.second_entry_time = perf_counter()

            self.update_limit = (self.second_entry_time - self.first_entry_time)
            # print("update",self.update_limit)
        index = self.letter_to_label_index[value[0]]
        let = value[0] + ": "

        if 's' in let or 'y' in let or "Y" in let or "S" in let:
            func = str
        else:
            func = float

        array_form = parse_letter_in_string(value, func)  # parse into array

        if 'y' in let or "Y" in let:
            self.pack_wide_box.setText("Pack wide: " + str(bin(int(array_form[0], 16))[2:]))
            self.pack_wide_info = bin(int(array_form[0], 16))[2:]

        else:
            if 'S' in let or 's' in let:
                new = []
                for i in array_form:
                    try:
                        new.append(bin(int(i, 16))[2:])
                    except:
                        pass

                if self.pack_wide_info != "":
                    toappend = ""
                    for x in self.pack_wide_box.text():
                        if x in ['1', '0']:
                            toappend += x
                    new.append(toappend)

                array_form = new

            if value[0] in ['v', 'V']:
                self.volt_num_of_entries+=1
                if self.first_voltage_time is None:
                    self.first_voltage_time = perf_counter()
                elif self.second_voltage_time is None:
                    self.second_voltage_time = perf_counter()

                    self.voltage_update_limit = (self.second_voltage_time - self.first_voltage_time)

                    # print("voltage update", self.voltage_update_limit)
                new_value = []
                # value+="\nSlopes: "
                for i in range(len(self.boxes)):
                    box = self.boxes[i]

                    if self.alpha == 1:
                        cur_slope = round(self.get_slopes(arry=box.all_voltages,x_interval=self.voltage_update_limit,
                                                      how_many=5),
                                         self.significant_digits

                                         )

                        spacnum = 0
                        if cur_slope>0:
                            spacnum = 1


                        self.check_box_labels[i].setText(
                            "m = "+ spacnum*" " + str(signi_digs(cur_slope, self.significant_digits)) +
                            f" (Module {i})"
                        )

                        self.check_box_labels[i].update()

                        print("in volt")
                        # self.check_box_labels[i].update()
                        # print(f"yes{str(cur_slope)} ::: {str(len(box.all_voltages))}")


                self.total_voltage = 0
                self.minvolt = 100000
                self.maxvolt = 0

                for i in range(len(array_form)):
                    self._grapher_manager.update_all_states(module_number=i,
                                                            values = self.boxes[i].all_voltages,
                                                            event = self.event,
                                                            num_of_entries=self.volt_num_of_entries,
                                                            update_interval_in_seconds=self.voltage_update_limit,
                                                            max_entries = self.entry_limit)




                    try:

                        self.total_voltage += float(array_form[i]) / 10000
                        new_value.append(float(array_form[i]) / 10000)


                        if self.minvolt>float(array_form[i])/10000:
                            self.minvolt = float(array_form[i]) / 10000
                            self.minvolt_module = i+1

                        if self.maxvolt < float(array_form[i]) / 10000:
                            self.maxvolt = float(array_form[i]) / 10000
                            self.maxvolt_module = i+1
                    except Exception as e:
                        print(e)

                array_form = new_value
                self.pack_wide_voltage.labels[0].setText(
                    self.spacnow*" "+"Total Voltage: " + str(round(self.total_voltage, self.significant_digits)))

                self.pack_wide_voltage.labels[1].setText(
                    self.spacnow*" " +"Min V: " + str(signi_digs(self.minvolt,self.significant_digits)) + f" (Module {self.minvolt_module})"    )

                self.pack_wide_voltage.labels[2].setText(
                    self.spacnow*" " + "Max V: "  +str(signi_digs(self.maxvolt,self.significant_digits) + f" (Module" +
                                                                                       f" {self.maxvolt_module})"
                                    ))

                self.pack_wide_voltage.voltage_bar.setValue(100 * self.total_voltage / (4.2 * 32))


            if value[0] in ['t','T']:
                self.temp_num_of_entries += 1
                self.mintemp = 100000
                self.maxtemp = 0

                for i in range(len(array_form)):


                    self._temp_grapher_manager.update_all_states(module_number=i,
                                                            values=self.boxes[i].all_temps,
                                                            event=self.event,
                                                            num_of_entries=self.temp_num_of_entries,
                                                            update_interval_in_seconds=self.voltage_update_limit,
                                                            max_entries=self.entry_limit)
                    try:

                        if self.mintemp > float(array_form[i]):
                            self.mintemp = float(array_form[i])
                            self.mintemp_module = i + 1

                        if self.maxtemp < float(array_form[i]):
                            self.maxtemp = float(array_form[i])
                            self.maxtemp_module = i + 1

                    except Exception as e:
                        print(e)


                    if self.alpha==2:
                        box = self.boxes[i]
                        cur_slope = round(self.get_slopes(arry=box.all_temps,x_interval=self.voltage_update_limit,
                                                      how_many=5),
                                         self.significant_digits

                                         )


                        timess=0
                        if cur_slope<0:
                            timess=1

                        self.check_box_labels[i].setText(
                            "m = " + timess*" " + str(cur_slope) + f" (Module {i})"
                        )

                        self.check_box_labels[i].update()

                self.pack_wide_voltage.labels[3].setText(" "*self.spacnow+
                    "Min T: " +  str(signi_digs(self.mintemp,
                                                                                self.significant_digits)) + f" ("
                                                                                                              f"Module"
                                                                                      f" {self.mintemp_module})")

                self.pack_wide_voltage.labels[4].setText(" "*self.spacnow+
                    "Max T: "+ str(signi_digs(self.maxtemp,
                                                                                  self.significant_digits)) + f" ("
                                                                                                              f"Module {self.maxtemp_module})")

            if self.alpha == 1 and ("v" in let or "V" in let):
                self.draw_voltage_graph()
            elif self.alpha == 2 and ("t" in let or "T" in let):
                self.draw_temp_graph()

            elif self.alpha == 3 and ("s" in let or "S" in let):

                self.update_status_table(array_form, let)
                self.show_status_table()

            self.update_values(array_of_values=array_form, label_index=index, letter=let)

        self.add_to_terminal(line=value)


def open_app():
    import sys

    sys._excepthook = sys.excepthook

    def exception_hook(exctype, value, traceback):

        #print(exctype, value, traceback)
        sys._excepthook(exctype, value, traceback)
        sys.exit(1)


    sys.excepthook = exception_hook

    app = QtWidgets.QApplication(sys.argv)  # initialize application

    MainWindow = QtWidgets.QMainWindow()  # init main window

    ui = Ui_MainWindow(MainWindow)

    ui.MainWindow.showMaximized()
    # ui.MainWindow.show()

    sys.exit(app.exec_())


if __name__ == '__main__':
    open_app()
