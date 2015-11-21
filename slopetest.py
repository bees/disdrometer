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


#def calibrate():
#    temp = []
#    while True:
#        for c in ser.read():
#            if chr(c) == 'd':
#                return
#            elif chr(c) != '\r' and chr(c) != '\n':
#                temp.append(c)
#            else temp.:
#                print(''.join(temp))
#                temp = []


if __name__ == '__main__':
    ser = serial.Serial(
            port='/dev/ttyACM0',\
            baudrate=9600,\
            parity=serial.PARITY_NONE,\
            stopbits=serial.STOPBITS_ONE,\
            bytesize=serial.EIGHTBITS,\
            timeout=0)

    print("connected to: " + ser.portstr)
    #ser.write(serial.to_bytes('s'))


    #calibrate()

    get_data()
    ser.close()

