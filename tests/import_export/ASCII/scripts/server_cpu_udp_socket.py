#!/usr/bin/python

import socket
import psutil

HOST = 'localhost'
PORT = 1027
ADDR = (HOST,PORT)
serv = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

while True:
  cpu_percent = str(psutil.cpu_percent(interval=0.5))
  serv.sendto(cpu_percent, ADDR)
  print 'written ' + cpu_percent
  
