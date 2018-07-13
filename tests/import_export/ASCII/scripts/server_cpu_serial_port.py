#!/usr/bin/python

import psutil, os, pty, serial

master, slave = pty.openpty()
s_name = os.ttyname(slave)
m_name = os.ttyname(master)
print 'slave serial port ' + s_name
#print 'master serial port ' + m_name

s = serial.Serial(s_name, 9600)

while True:
  cpu_percent = str(psutil.cpu_percent(interval=0.5))
  s.write(cpu_percent)
  print 'written ' + cpu_percent
