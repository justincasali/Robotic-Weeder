import serial

RX_COORD = 0x01
TX_CAL = 0x01

c1 = b'\x00\xc6\x018'
c2 = b'\x01\xed\x017'
c3 = b'\x01\xe5\x00l'
c4 = b'\x00\xc2\x00\x7f'

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
micro.write(c2)

wait_for_request()
micro.write(c3)

wait_for_request()
micro.write(c4)
