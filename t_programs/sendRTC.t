variables {
    // Base channel. CAN messages will be sent on channel ch
    const int ch = 0;

    // The singleshot timer is used to get a delay before the first RTC CAN message
    Timer singleshot;

    // The periodic timer is then used between each RTC CAN message
    Timer periodic;

    // The message id of the sent RTC CAN messages
    int msgId = 750;

    // The CAN message to be sent
    CanMessage msg;
}

// Function to convert int to an array of 8 bytes
void intToByteArray(int value, char byteArray[]) {
    byteArray[0] = (value & 0xFF);
    byteArray[1] = ((value >> 8) & 0xFF);
    byteArray[2] = ((value >> 16) & 0xFF);
    byteArray[3] = ((value >> 24) & 0xFF);
}

// Function to get the seconds of the tm structure
// tm_mon is 0 to 11
int tmToEpochSeconds(tm timeNow) {
    float seconds = 0;
    seconds += timeNow.tm_sec;
    seconds += timeNow.tm_min * 60;
    seconds += timeNow.tm_hour * 3600;
    seconds += timeNow.tm_mday * 86400;
    seconds += timeNow.tm_mon * 2592000;
    seconds += (timeNow.tm_year - 70) * 31536000;

    return (int)seconds;
}

on Timer periodic {
    tm currTime;
    timeGetDate(currTime);                          // Get the time struct containing date time information
    int seconds = tmToEpochSeconds(currTime);     // Convert tm struct to epoch seconds
    
    byte canData[8] = {0, 0 ,0, 0, 0, 0, 0, 0};   // Create the byte array all zeros
    intToByteArray(seconds, canData);             // Convert the float to a byte array

    // After using the current CAN message id, increment before next use
    msg.data  = canData;

    // Send CAN message
    canWrite(ch, msg);
}

on start {
    printf("Starting RTC timestamp script\n");

    // Setup ONE CAN channel to send messages. This is hard coded to CH1
    // This will override the settings in the binary configuration,
    // most notably the channels will no longer be in silent mode.
    canBusOff(ch);
    canSetBitrate(ch, canBITRATE_500K);
    canSetBusOutputControl(ch, canDRIVER_NORMAL);
    canBusOn(ch);

    periodic.timeout   = 1000 * 60;  // One second period

    // Start the periodic timer to send the RTC timestamp
    msg.id    = msgId;
    msg.flags = 0;
    msg.dlc   = 8;
    timerStart(periodic);
    printf("Start periodic transmission\n");
}

on stop {
    printf("Stopping script\n");
    canBusOff(ch);
}