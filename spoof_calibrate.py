#!/usr/local/bin/python3
import serial

RX_COORD = 0x01
TX_CAL = 0x01

c1 = bytes([0x01, 0xF4, 0x02, 0xA3])
c2 = bytes([0x06, 0x27, 0x02, 0xA3])
c3 = bytes([0x01, 0xF4, 0x08, 0x1B])
c4 = bytes([0x06, 0x27, 0x08, 0x1B])

def wait_for_request():
    while True:
        if (micro.in_waiting > 0):
            if (micro.read() == bytes([RX_COORD])):
                break

micro = serial.Serial('/dev/cu.usbserial-A105RZEV')

# Send go to coord state byte
micro.write(bytes([TX_CAL]))

wait_for_request()
micro.write(c1)

wait_for_request()
micro.write(c1)

wait_for_request()
micro.write(c2)

wait_for_request()
micro.write(c3)
