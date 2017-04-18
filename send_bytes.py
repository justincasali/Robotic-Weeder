#!/usr/local/bin/python3
import serial
from sys import argv

micro = serial.Serial('/dev/cu.usbserial-A105RZEV')

chain = list()

for x in argv[1::]:
    chain.append(int(x, 16) & 0xff)

byte_array = bytes(chain)
micro.write(byte_array)

print("array sent:", byte_array)
micro.close()
