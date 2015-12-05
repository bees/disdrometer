import serial
import matplotlib.pyplot as plt
import csv
from collections import namedtuple

Droplet = namedtuple('Droplet', ['amplitude', 't', 'ts'])




#this will store the line


def get_data():
    print("listening for slopes:")
    temp = 0
    data = []
    while True:
        for c in ser.read():
            if chr(c) == 'd':
                return
            elif chr(c) != '\r' and chr(c) != '\n' and chr(c) !='-':
                temp = temp*10 + int(chr(c))
            elif temp != 0:
                print(temp)
                temp = 0

def serial_start():
    while True:
        for c in ser.read():
            if chr(c) == 's':
                return


def calibrate():
    print("Doing calibration routine")
    ser.write(serial.to_bytes([99]))
    serial_start()
    temp = 0
    while True:
        for c in ser.read():
            if chr(c) == 'd':
                return
            elif chr(c) != '\r' and chr(c) != '\n' and chr(c) !='-':
                temp = temp*10 + int(chr(c))
            elif temp != 0:
                print(temp)
                temp = 0



if __name__ == '__main__':
    ser = serial.Serial(
            port='/dev/ttyACM0',\
            baudrate=9600,\
            parity=serial.PARITY_NONE,\
            stopbits=serial.STOPBITS_ONE,\
            bytesize=serial.EIGHTBITS,\
            timeout=0)

    ser.flushInput()
    ser.flushOutput()

    print("connected to: " + ser.portstr)


    calibrate()

    get_data()
    ser.close()

