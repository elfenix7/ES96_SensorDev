#!/usr/bin/env python
# datalogger.py
# This script repeatedly reads in a line from the serial port,
# then saves it to a text file with a filename given by the following format:
# yyyy-mm-dd_hh-mm-ss.txt
import sys
import time
import serial
import RPi.GPIO as GPIO

INIT_SUCCESS = "Good to go!"

ser = serial.Serial (
        port='/dev/ttyUSB0',
        baudrate=9600,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS
)


# cleanup GPIO and close serial port; call this before exiting
def cleanup():
    global GPIO, ser
    GPIO.cleanup()
    ser.close()


# cleanup and print an (optional) error message before exiting
def die(errormsg="", exitcode=0):
    cleanup()
    if errormsg:
        print(errormsg, file=sys.stderr)
    sys.exit(exitcode)


if __name__ == "__main__":
    # initialization
    GPIO.setmode(GPIO.BOARD) #TODO: use BOARD or BCM mode?
    ser.close() #serial port may already be open; reopening resets the Arduino
    ser.open()

    # verify successful Arduino+sensor startup
    status = ser.readline()
    status = status.strip().decode("utf-8")
    if (status != INIT_SUCCESS):
        die("Error: sensor initialization failed!", 1)

    # main loop
    while True:
        try:
            rawdata = ser.readline()
            data = rawdata.decode("utf-8")
            
            timestamp = time.strftime("%Y-%m-%d_%H-%M-%S", time.localtime())
            filename = timestamp + ".txt"
            
            outfile = open(filename, 'w')
            outfile.write(data)
            outfile.close()
            
        except:
            cleanup()
            raise
            
