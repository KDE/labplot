#!/usr/bin/python3

from socket import *
import psutil, time

HOST = 'localhost'
PORT = 1027
ADDR = (HOST,PORT)

serv = socket(AF_INET, SOCK_DGRAM)
serv.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
while True:
  message = str(psutil.cpu_percent()) + " " + str(psutil.cpu_freq().current) +  "\n"
  serv.sendto(message.encode(), ADDR)
  print('written ' + message)
  time.sleep(0.5)
