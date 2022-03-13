#!/usr/bin/python

import psutil, serial, time
from subprocess import Popen

Popen(["socat", "-d", "-d", "PTY,raw,echo=0,link=./serial_in", "PTY,raw,echo=0,link=./serial_out"])
time.sleep(1)

# rtscts=True,dsrdtr=True
s = serial.Serial("./serial_in", 9600)
print(s)

while True:
  cpu_percent = str(psutil.cpu_percent(interval=2.))
  s.write(cpu_percent)
  print('written ' + cpu_percent)
