import serial
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

# Function to stall until Control Byte is read
def wait_for_request(CB):
    while True:
        if (micro.in_waiting > 0):
            if (micro.read() == CB):
                break

# Function to convert coordinate into byte
def coord_to_bytes(x, y):
    return bytes([x >> 8 & 0xff, x & 0xff, y >> 8 & 0xff, y & 0xff])

# Fake calibration points used for spoofing
c1 = coord_to_bytes(198, 312)
c2 = coord_to_bytes(493, 311)
c3 = coord_to_bytes(485, 108)
c4 = coord_to_bytes(194, 217)

# Open serial port
micro = serial.Serial('/dev/cu.usbserial-A105RZEV')

# SYSTEM WHILE LOOP STARTS HERE

# Send go to calibration state byte
micro.write(TX_Calibration)

# Wait until mc ask to send calibration location 1
wait_for_request(RX_Coordinate)
sleep(1)
micro.write(c1)

# Wait until mc ask to send calibration location 2
wait_for_request(RX_Coordinate)
sleep(1)
micro.write(c2)

# Wait until mc ask to send calibration location 3
wait_for_request(RX_Coordinate)
sleep(1)
micro.write(c3)

# Wait until mc ask to send calibration location 4
wait_for_request(RX_Coordinate)
sleep(1)
micro.write(c4)

# Wait a bit before going into halt state
sleep(3)

# Send halt signal
micro.write(TX_Halt)
wait_for_request(RX_Location)

# LOCATION STATE (Main algorithim goes here)
sleep(3)
# Write number of dandelions
micro.write(TX_Num_Ready)
micro.write(bytes([2]))

# Write coord of first dandelion
micro.write(TX_Coord_Ready)
micro.write(c1)

# Write coord of second dandelion
micro.write(TX_Coord_Ready)
micro.write(c2)

# VERIFY STATE (Verification algorithim goes here)
wait_for_request(RX_Confirmation)
sleep(1)
micro.write(TX_Pseudocation)

wait_for_request(RX_Confirmation)
sleep(1)
micro.write(TX_Recalibration)

# Verification fails, must Recalibrate here
wait_for_request(RX_Coordinate)
sleep(1)
micro.write(c1)

wait_for_request(RX_Coordinate)
sleep(1)
micro.write(c2)

wait_for_request(RX_Coordinate)
sleep(1)
micro.write(c3)

wait_for_request(RX_Coordinate)
sleep(1)
micro.write(c4)

# Reverify for failed coordinate
wait_for_request(RX_Confirmation)
sleep(1)
micro.write(TX_Pseudocation)
