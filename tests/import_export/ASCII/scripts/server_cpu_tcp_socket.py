#!/usr/bin/python

import socket, psutil

HOST = 'localhost'
PORT = 1027
ADDR = (HOST,PORT)
serv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

serv.bind(ADDR)
serv.listen(1)

print 'listening ...'

while True:
  conn, addr = serv.accept()
  print 'client connected ... ', addr
  cpu_percent = str(psutil.cpu_percent())
  conn.send(cpu_percent)
  print 'written ' + cpu_percent
  conn.close()
  print 'client disconnected'
