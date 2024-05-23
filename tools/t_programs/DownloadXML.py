# Purpose   : Script to load XML file onto Kvaser Memorator Pro 2xHS x2
# Author    : Aarjav Jain
# Date      : 2024-05-23
# 

from canlib import canlib
from canlib import EAN, Device
from canlib import kvamemolibxml
from canlib import kvmlib

# Constants 
CH_NUM                  = 0                      # Assumes device is connected to channel 0 
XML_FILE_NAME           = "sendRTC.xml"          # Path to XML file in same directory as this code

# Returns the EAN using the .ChannelData.card_upc_no attribute
def getEAN():
    ch = CH_NUM
    chdata = canlib.ChannelData(ch)
    return chdata.card_upc_no

# Function to load the XML binary file onto memorator
def loadBinaryXML():
    ean = getEAN()
    print(f"EAN: {ean}")

    dev = Device.find(ean=EAN(ean))                         # Get a device object to then download to

    with open(XML_FILE_NAME, 'r') as myfile:                # Get XML file string
        config_xml = myfile.read()

    config_lif = kvamemolibxml.kvaXmlToBuffer(config_xml)   # Convert XML string to bytes

    with kvmlib.openDevice(dev.channel_number()) as memo:   # Write XML binary to memorator
        memo.write_config(config_lif)


def main():
    loadBinaryXML()

if __name__ == "__main__":
    main()
    