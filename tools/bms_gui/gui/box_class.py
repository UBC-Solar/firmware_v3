from PyQt5 import QtCore, QtGui, QtWidgets

# A class to store all of the information. Each bar and label will be it's own box_info class
# All coordinates fed to this class is already adjusted relative to the screen size, so we do not have to
# adjust any of the coordinates.
class box_info:
    def __init__(self, centralwidget, x, y, w, h, module_number, height,width, spacing =15, text_list=None,
                 voltage_text="V: X.XX"):


        self.module_number = module_number
        self.color,self.word_color = self.generate_color(seed = module_number)

        if text_list is None:
            text_list = [ "v: xx.xxxx", "t: xx.xxxx", "s: xx.xxxx", "BAL", "Module " + str(module_number+1) ,]


        #self.font_scale=768
        self.height=height
        self.width=width
        self.original_width=1368
        self.original_height=768
        self.lim = 300
        self.big_lim = 1000000
        self.x=x
        self.y=y
        self.w=w
        self.h=h
        self.labels=[]
        self.past_temps = []
        self.past_voltages = []

        self.all_temps = []
        self.all_voltages = []
        self.generate_voltage_bar(centralwidget) #generating the bar
        self.generate_all_labels(centralwidget,text_list,base_spacing=spacing)

    def relx(self,number):
        return (number/self.original_width)*self.width
    def rely(self,number):
        return (number/self.original_height)*self.height

    @staticmethod
    def generate_color(seed):
        final=['#696969', '#228b22', '#800000', '#808000', '#483d8b', '#008b8b', '#cd853f', '#4682b4', '#000080', '#32cd32', '#7f007f', '#8fbc8f', '#b03060', '#ff0000', '#ff8c00', '#ffd700', '#00ff00', '#8a2be2', '#dc143c', '#00ffff', '#0000ff', '#adff2f', '#ff00ff', '#1e90ff', '#fa8072', '#b0e0e6', '#90ee90', '#ff1493', '#7b68ee', '#f5deb3', '#ee82ee', '#ffb6c1']
        words=final

        return final[seed],words[seed]

    def generate_voltage_bar(self, centralwidget):
        #generates voltage bar

        self.voltage_bar = QtWidgets.QProgressBar(centralwidget)
        self.voltage_bar.setEnabled(True)
        self.voltage_bar.setGeometry(QtCore.QRect(self.x, self.y, self.w, self.h))
        self.voltage_bar.setProperty("value", 0)
        self.voltage_bar.setOrientation(QtCore.Qt.Vertical)
        self.voltage_bar.setTextVisible(False)
        self.voltage_bar.setObjectName("voltage_bar")

    def general_label(self,centralwidget,text, vertical_spacing=10,):
        # vertical_spacing=(vertical_spacing/self.original_width)*self.width
        #generates 1 label
        sheet="text-align:center;"
        v_label = QtWidgets.QLabel(centralwidget)
        v_label.setText(text)
        v_label.setStyleSheet(sheet)

        a=self.rely(11)
        aa=self.relx(11)
        if a<aa:
            final=a
        else:
            final=aa
        v_label.setFont(QtGui.QFont("Segoe UI", final))
        v_label.setGeometry(self.x-self.relx(10),
                            self.y+self.h+vertical_spacing,
                            self.w+self.relx(30),
                            self.rely(15))

        v_label.setAlignment(QtCore.Qt.AlignCenter)
        return v_label

    def generate_all_labels(self,centralwidget,text_list,base_spacing):
        #Generates all four labels
        base_spacing=self.rely(base_spacing)
        for i in range(len(text_list)):
            spacing= base_spacing*i
            if i==len(text_list)-1 and base_spacing==10:
                spacing+=5

            self.labels.append(

                self.general_label(centralwidget,text_list[i],vertical_spacing=spacing)
            )


    def change_voltage(self,value):
        #changes colour for voltage bar
        if value<2:
            color="red"
        elif value<4:
            color="yellow"
        else:
            color="green"


        complete="QProgressBar::chunk {\nbackground-color:"+color+";}"
        self.voltage_bar.setStyleSheet(complete)
        self.voltage_bar.setValue(value*100/4.2)
    def hide_all(self):
        for k in self.labels:
            k.hide()

        self.voltage_bar.hide()

    def show_all(self):
        for k in self.labels:
            k.show()

        self.voltage_bar.show()

    def update_past_temps(self, new_temp_to_add):
        if len(self.past_temps)>self.lim:
            self.past_temps.pop(0)

        if len(self.all_temps)>self.big_lim:
            self.all_temps.pop(0)
        self.all_temps.append(new_temp_to_add)


        self.past_temps.append(new_temp_to_add)


    def update_past_voltages(self, new_voltage_to_add):
        if len(self.past_voltages)>self.lim:
            self.past_voltages.pop(0)

        if len(self.all_voltages)>self.big_lim:
            self.past_voltages.pop(0)
        self.all_voltages.append(new_voltage_to_add)
        self.past_voltages.append(new_voltage_to_add)

