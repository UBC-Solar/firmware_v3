# Purpose: Download XML config to the memorator
# Author: Aarjav Jain
# Date: 2024-05-24
# 

from canlib import EAN, Device
from canlib import kvamemolibxml
from canlib import kvmlib
from canlib import canlib
import canlib.kvaMemoLibXml as kvaMemoLibXml
import os


CAN_CHANNEL_1           = 0
XML_CONFIG_FULL_PATH    = "<ENTER_YOUR_FULL_ABSOLUTE_PATH_TO_XML_CONFIG_FILE>"    # Example: "C:/Users/my_username/Documents/sendRTC.xml"


def getEAN():
    ch = canlib.ChannelData(0)
    return ch.card_upc_no

def downloadXML():
    ean = getEAN()
    dev = Device.find(ean=EAN(ean))

    # Read in the XML configuration file
    with open(XML_CONFIG_FULL_PATH, 'r') as myfile:
        config_xml = myfile.read()

    # Convert the XML configuration to a binary configuration
    config_lif = kvamemolibxml.kvaXmlToBuffer(config_xml)

    # Open the memorator and write the binary configuration
    with kvmlib.openDevice(dev.channel_number()) as memo:
        memo.write_config(config_lif)

def validateXML():
    xl = kvaMemoLibXml.kvaMemoLibXml()
    print("kvaMemoLibXml version: v" + str(xl.dllversion()))

    # Read in the XML configuration file
    with open(XML_CONFIG_FULL_PATH, 'r') as myfile:
        config_xml = myfile.read()

    # Validate the XML configuration
    xl.kvaXmlValidate(config_xml)

    # Get number of validation messages
    (countErr, countWarn) = xl.xmlGetValidationStatusCount()
    print("Errors: %d, Warnings: %d" % (countErr, countWarn))

    # If we have any validation errors, print those
    if countErr != 0:
        code = -1
        while code != 0:
            (code, text) = xl.xmlGetValidationError()
            print("%d: %s" % (code, text))

    # If we have any validation warnings, print those
    if countWarn != 0:
        code = -1
        while code != 0:
            (code, text) = xl.xmlGetValidationWarning()
            print("%d: %s" % (code, text))

    # Exit if we had any validation errors or warnings
    if countErr != 0 or countWarn != 0:
        raise Exception('Please fix validation Errors/Warnings.')


def main():
    validateXML()
    downloadXML()
    print("XML config downloaded successfully!")

if __name__ == "__main__":
    main()
