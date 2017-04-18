#!/usr/local/bin/python3
import serial
from sys import argv

micro = serial.Serial('/dev/cu.usbserial-A105RZEV')

while True:
    if (micro.in_waiting > 0):
        print(micro.read()[0])
