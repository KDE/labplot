#!/usr/bin/python

from socket import *
import psutil

HOST = 'localhost'
PORT = 1027
ADDR = (HOST,PORT)
client = socket(AF_INET, SOCK_DGRAM)
client.bind(ADDR)
while True:
  data, addr = client.recvfrom(1024)
  print('read ' + data, addr)


