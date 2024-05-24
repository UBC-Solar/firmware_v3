# Purpose   : Script to load XML file onto Kvaser Memorator Pro 2xHS x2
# Author    : Aarjav Jain
# Date      : 2024-05-23
# 

from canlib import canlib
from canlib import EAN, Device
from canlib import kvamemolibxml
from canlib import kvmlib
import canlib.kvaMemoLibXml as kvaMemoLibXml

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

def performValidation():
    xl = kvaMemoLibXml.kvaMemoLibXml()
    print("kvaMemoLibXml version: v" + xl.getVersion())

    with open(XML_FILE_NAME, 'r') as myfile:
        config_xml = myfile.read()

    xl.kvaXmlValidate(config_xml)

    (countErr, countWarn) = xl.xmlGetValidationStatusCount()
    print("Errors: %d, Warnings: %d" % (countErr, countWarn))

    if countErr != 0:
        code = -1
        while code != 0:
            (code, text) = xl.xmlGetValidationError()
            print("%d: %s" % (code, text))

    if countWarn != 0:
        code = -1
        while code != 0:
            (code, text) = xl.xmlGetValidationWarning()
            print("%d: %s" % (code, text))

    if countErr != 0 or countWarn != 0:
        raise Exception('Please fix validation Errors/Warnings.')

def main():
    loadBinaryXML()             # Load XML file onto memorator
    performValidation()         # Check for any errors

if __name__ == "__main__":
    main()
