#!/usr/bin/python

from socket import *
import psutil

HOST = '127.0.0.1'
PORT = 59542
ADDR = (HOST,PORT)

serv = socket(AF_INET, SOCK_DGRAM)
serv.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
while True:
  cpu_percent = str(psutil.cpu_percent(interval=0.5))
  serv.sendto(cpu_percent.encode(), ADDR)
  print('written ' + cpu_percent, ADDR)
