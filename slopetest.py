import serial
import matplotlib.pyplot as plt
import csv



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


if __name__ == '__main__':
    ser = serial.Serial(
            port='/dev/ttyACM0',\
            baudrate=9600,\
            parity=serial.PARITY_NONE,\
            stopbits=serial.STOPBITS_ONE,\
            bytesize=serial.EIGHTBITS,\
            timeout=0)

    print("connected to: " + ser.portstr)

    get_data()
    ser.close()

