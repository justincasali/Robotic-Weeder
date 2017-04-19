import serial
from tk1 import *
from time import sleep

# RX Control Bytes
RX_Coordinate    = bytes([0x01])
RX_Roaming       = bytes([0x02])
RX_Location      = bytes([0x03])
RX_Shutdown      = bytes([0x04])
RX_Positioning   = bytes([0x05])
RX_Confirmation  = bytes([0x06])

# TX Control Bytes
TX_Calibration   = bytes([0x01])
TX_Halt          = bytes([0x02])
TX_Recalibration = bytes([0x03])
TX_Pseudocation  = bytes([0x04])
TX_Num_Ready     = bytes([0x05])
TX_Coord_Ready   = bytes([0x06])

def wait_for_request(CB):
    while True:
        if (micro.in_waiting > 0):
            if (micro.read() == CB):
                break

def coord_to_bytes(x, y):
    return bytes([x >> 8 & 0xff, x & 0xff, y >> 8 & 0xff, y & 0xff])

def calibrate():
    for i in range(4):
        wait_for_request(RX_Coordinate)
        x, y = get_red()
        micro.write(coord_to_bytes(x, y))

# MAIN START

micro = serial.Serial('/dev/ttyUSB0')

micro.write(TX_Calibration)
calibrate()

while True:

    if dandelions_exist():

        micro.write(TX_Halt)
        wait_for_request(RX_Location)

        x_list, y_list = locate()
        count = len(x_list)

        micro.write(TX_Num_Ready)
        micro.write(bytes([count]))

        for i in range(count):
            micro.write(TX_Coord_Ready)
            micro.write(coord_to_bytes(x_list[i], y_list[i])

        dand = 0
        while (dand < count):
            wait_for_request(RX_Confirmation)

            if verify(x_list[dand], y_list[dand]):
                micro.write(TX_Pseudocation)
                dand = dand + 1
            else:
                calibrate()

    sleep(3)

