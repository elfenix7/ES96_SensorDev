#!/usr/bin/env python
# datalogger.py
# This script repeatedly reads in a line from the serial port,
# then saves it to a text file with a filename given by the following format:
# yyyy-mm-dd_hh-mm-ss.txt
import os
import sys
import time
import serial
#import RPi.GPIO as GPIO
import requests
from requests.exceptions import HTTPError
import json
from json_payload import *

DATA_DIR = "/home/pi/avocado_data"

NUM_SCAN_FIELDS = 21    # number of expected fields per scan

ser = serial.Serial (
        port='/dev/ttyACM0',
        baudrate=115200,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS
)


# cleanup GPIO and close serial port; call this before exiting
def cleanup():
    #global GPIO
    global ser
    #GPIO.cleanup()
    ser.close()


# cleanup and print an (optional) error message before exiting
def die(errormsg="", exitcode=0):
    cleanup()
    if errormsg:
        print(errormsg, file=sys.stderr)
    sys.exit(exitcode)


if __name__ == "__main__":
    # initialization
    #GPIO.setmode(GPIO.BOARD) #TODO: use BOARD or BCM mode?
    os.chdir(DATA_DIR)
    ser.close()
    ser.open()

    # main loop
    while True:
        try:
            scandata = ser.readline().decode("utf-8")
            vals = scandata.split(",")
            if len(vals) < NUM_SCAN_FIELDS:
                print("Not enough data received! Expected %d fields, received %d" % NUM_SCAN_FIELDS, len(vals), file=sys.stderr)
                continue
            
            # break out received data into individual measurements
            scan_id = vals[0]
            stage = int(vals[1])
            light_spectrum = list(map(float, vals[2:20]))
            sound_atten = float(vals[20].split(" ", 1)[0])
            # TODO: get impedance values
            # impedance_sweep = list(map(float, vals[20:]))
            timestamp = time.strftime("%Y-%m-%d_%H-%M-%S", time.localtime())
            
            # debug: print received values
            print("scan_id: %s" % scan_id)
            print("stage: %d" % stage)
            print("light_spectrum: %s" % light_spectrum)
            print("sound_atten: %f" % sound_atten)

            req_payload = populate_payload(scan_id=scan_id, time=timestamp, light_spectrum=light_spectrum, sound_atten=sound_atten, stage=stage)
            
            # save scan results
            filename = scan_id + "_" + timestamp + ".json"
            outfile = open(filename, 'w')
            outfile.write(json.dumps(req_payload))
            outfile.close()
            
            # upload scan to database
            try:
                response = requests.post(url, data=json.dumps(req_payload), headers=headers)
                print(response.text.encode('utf8'))
                response.raise_for_status
            except HTTPError as http_err:
                print(f'HTTP error occured: {http_err}')
            except Exception as err:
                print(f'Other error occured: {err}')
            else:
                print('Success!')
            
        except:
            cleanup()
            raise
            
