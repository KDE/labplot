#!/usr/bin/python

import serial

#import serial.tools.list_ports
#print list(serial.tools.list_ports.comports())

s = serial.Serial("./serial_out", 9600,timeout=2)
print(s)

while True:
  x = s.read(3)
  print('read ' + x)
