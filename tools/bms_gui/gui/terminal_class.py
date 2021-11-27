from PyQt5 import QtWidgets

class terminal(QtWidgets.QScrollArea):

    # This class is bascially a label inside a scrollarea.
    def __init__(self, x, y, w, h,  window):
        QtWidgets.QScrollArea.__init__(self, window)

        # making widget resizable
        self.setWidgetResizable(True)


        #setting the position
        self.setGeometry(x,y,w,h)

        # making qwidget object
        content = QtWidgets.QWidget(self)
        self.setWidget(content)

        # vertical box layout
        lay = QtWidgets.QVBoxLayout(content)

        # creating label
        self.label = QtWidgets.QLabel(content)

        # setting alignment to the text
        # self.label.setAlignment(Qt.AlignLeft | Qt.AlignTop)

        # making label multi-line
        self.label.setWordWrap(True)

        # adding label to the layout
        lay.addWidget(self.label)

    # the setText method
    def setText(self, text):
        # setting text to the label
        self.label.setText(text)
