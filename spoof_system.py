import serial
from time import sleep

RX_COORD = 0x01
TX_CAL = 0x01

c1 = b'\x00\xc6\x018'
c2 = b'\x01\xed\x017'
c3 = b'\x01\xe5\x00l'
c4 = b'\x00\xc2\x00\x7f'

def wait_for_request(CB):
    while True:
        if (micro.in_waiting > 0):
            if (micro.read() == bytes([CB])):
                break

micro = serial.Serial('/dev/cu.usbserial-A105RZEV')

# Send go to coord state byte
micro.write(bytes([TX_CAL]))

wait_for_request(TX_CAL)
sleep(1)
micro.write(c1)

wait_for_request(TX_CAL)
sleep(1)
micro.write(c2)

wait_for_request(TX_CAL)
sleep(1)
micro.write(c3)

wait_for_request(TX_CAL)
sleep(1)
micro.write(c4)

sleep(3)

# Send halt signal
micro.write(bytes([0x02]))
wait_for_request(0x03)

# LOCATION STATE
micro.write(bytes([0x05]))
micro.write(bytes([2]))

micro.write(bytes([0x06]))
micro.write(c1)

micro.write(bytes([0x06]))
micro.write(c2)

# VERIFTY STATE
wait_for_request(0x01)
# Verification algorithim
micro.write(bytes([0x04]))

wait_for_request(0x01)
micro.write(bytes([0x03]))

wait_for_request(TX_CAL)
sleep(1)
micro.write(c1)

wait_for_request(TX_CAL)
sleep(1)
micro.write(c2)

wait_for_request(TX_CAL)
sleep(1)
micro.write(c3)

wait_for_request(TX_CAL)
sleep(1)
micro.write(c4)

wait_for_request(0x01)
micro.write(bytes([0x04]))
