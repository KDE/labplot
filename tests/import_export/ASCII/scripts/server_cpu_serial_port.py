#!/usr/bin/python

import socket, psutil, os, pty, serial

master, slave = pty.openpty()
s_name = os.ttyname(slave)
print 'serial port ' + s_name

ser = serial.Serial(s_name)

while True:
  cpu_percent = str(psutil.cpu_percent(interval=0.5))
  ser.write(cpu_percent)
  print 'written ' + cpu_percent
