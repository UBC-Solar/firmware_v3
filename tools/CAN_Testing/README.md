**MVP Complete: How to Use the Script**

1. Go to the `firmware_v3` repository. Then go `tools` -> `CAN_Testing`

2. From here clone this folder in some workspace (Visual Studio Code for example)

3. Either use your own `requirements.json` or edit the provided one for your specific use case.
   a. For non-status messages (Ex. the CurrentSensor1 signal ) the list format is: [`min val`, `max val`, gap between messages in ms]. Note that a `DELTA` of +-100ms is hard coded.

    b. For status messages (Ex. the entire Faults message) the list format is: [`initial status`, `expected status`, gap between messages in ms]

    c. Note that the script assumes requirements.json is the default name, however, will prompt the user for the name of the JSON. Simply hit `Enter` if you want to use the default name

4. Add your own log files to the same directory. (Ex [log.txt](tools/CAN_Testing/log.txt))

5. Run `main.py`
6. If you want to Recieve messages type `r`

7. If you want your PASSED tests to show then type `y`. Otherwise `n`

8. Type in the name of your log file you added in **step 4**.

9. If your JSON file name is not `requirements.json` then simply hit `Enter`. Otherwise, type the name of your JSON requirements file.

10. A bunch of PASSED or FAILED messages will display

11. If you want the logs you added in **step 4**. to be converted to influxDB format and saved in a file called `influx_logs.txt` inside the same working directory type `y`.

12. Repeat from **step 6** except now you can type `s` to exit the script.
