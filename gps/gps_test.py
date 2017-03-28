import time
import serial
import pynmea2

GPSBAUD = 9600

def read(filename):
    f = open(filename)
    reader = pynmea2.NMEAStreamReader(f)

    while 1:
        for msg in reader.next():
          print(msg)


def read_serial(filename):
    com = None
    reader = pynmea2.NMEAStreamReader()

    while 1:

        if com is None:
          try:
            com = serial.Serial(filename, baudrate=GPSBAUD, timeout=5.0)
          except serial.SerialException:
            print('could not connect to %s' % filename)
            time.sleep(5.0)
            continue

        data = com.read(16)
        for msg in reader.next(data):
          print(msg)

if __name__ == "__main__":
    read_serial("/dev/ttyAMA0")
