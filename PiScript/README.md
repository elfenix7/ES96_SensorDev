This is the script to run on the raspberry pi for automated collection and saving of data from all the sensors.

`datalogger.py` first verifies that the sensor Arduino has started correctly.
Then, it indefinitely reads each incoming line from the serial port, then saves that line to a text file.
This text file is named using this timestamp format: `yyyy-mm-dd_hh-mm-ss.txt`
