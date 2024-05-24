variables {
    // Base channel. CAN messages will be sent on channel ch
    const int ch = 0;

    // The periodic timer is then used between each RTC CAN message
    Timer periodic;

    // The message id of the sent RTC CAN messages
    int msgId = 696;

    // The CAN message to be sent
    CanMessage msg;
}

void sendRTCMessage() {
    tm currTime;
    timeGetDate(currTime);                          // Get the time struct containing date time information
    
    byte canData[8] = {0, 0 ,0, 0, 0, 0, 0, 0};   // Create the byte array all zeros
    canData[0] = currTime.tm_sec & 0xFF;                    // seconds
    canData[1] = currTime.tm_min & 0xFF;                     // minutes
    canData[2] = currTime.tm_hour & 0xFF;                      // hours
    canData[3] = currTime.tm_mday & 0xFF;                      // day
    canData[4] = (currTime.tm_mon + 1) & 0xFF;                      // month
    canData[5] = (currTime.tm_year - 100) & 0xFF;                      // year
    canData[6] = currTime.tm_isdst & 0xFF;                      // daylight savings flag
    printf("Sending CAN message with epoch timestamp: %d Y %d month %d day %d hour %d min %d sec\n", canData[5], canData[4], canData[3], canData[2], canData[1], canData[0]);

    msg.data  = canData;

    // Send CAN message
    canWrite(ch, msg);
}

on Timer periodic {
    sendRTCMessage();

    printf("Periodic MsgId: %d\n", msg.id);
    if (!timerIsPending(periodic)) {
        printf("Timer done!");
    }
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

    periodic.timeout   = 500;  // One minute period. Periodic always starts after

    // Start the periodic timer to send the RTC timestamp
    msg.id    = msgId;
    msg.flags = 0;
    msg.dlc   = 8;
    sendRTCMessage();                 // Send the first message
    timerStart(periodic, FOREVER);
    printf("Start periodic transmission\n");
}

on stop {
    printf("Stopping script\n");
    canBusOff(ch);
}