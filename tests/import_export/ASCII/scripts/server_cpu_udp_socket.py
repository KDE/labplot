#!/usr/bin/python

from socket import *
import psutil

HOST = 'localhost'
PORT = 1027
ADDR = (HOST,PORT)

serv = socket(AF_INET, SOCK_DGRAM)
serv.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
while True:
  cpu_percent = str(psutil.cpu_percent(interval=0.5))
  serv.sendto(cpu_percent.encode(), ADDR)
  print('written ' + cpu_percent, ADDR)
