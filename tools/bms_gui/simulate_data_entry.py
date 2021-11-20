# A simulation file that sends random data serially to a virtual COM port

import serial
from reading_com_port import parse_information
from time import sleep
from random import randint,uniform

s = serial.Serial('COM7') #this writes to the virtual com port that is connected tot he com port opened in the GUI
# Normally, the data would come through whichever com port the cable is plugged into.
#Since this is only to simulate what would happen, there is no problem in hard coding the com port.
#Under normal circumstances, this script would never even run. This was only written for testing.


for i in range(100000010000001000000):
    asdf=["s","v","t","c","y"][i%5]
    potvolt=[1,2,3,4]


    if asdf=='s':
        t = "s: "
        for k in range(32):
            # module = ""
            # for p in range(17):
            #     module += str(choice([0, 1]))
            module=hex(randint(0,65535))[2:]

            t += module + " "
        t += "\n"

    elif asdf=='v':
        t="v: "
        for k in range(32):
            t+=str(int(uniform(0,4.2)*10000))[0:5]+" "
        t+="\n"


    elif asdf=='y':
        t='y: '
        module = hex(randint(0, 65535))[2:] #convertint to hex, it will be displayed normally in the actual gui

        t += module + "\n"

    else:
        t = parse_information.generate_random_voltages(x=asdf+": ")+"\n"


    print(i)
    s.write(t.encode())

    sleep(0.1)


s.close()
