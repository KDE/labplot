#!/usr/bin/python

import socket, psutil, os

ADDR = './local_socket'
serv = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

if os.path.exists(ADDR):
  print('socket exists. Removing it')
  os.remove(ADDR)

serv.bind(ADDR)
serv.listen(1)

print 'listening ...'

while True:
  conn, addr = serv.accept()
  print 'client connected ... ', addr
  cpu_percent = str(psutil.cpu_percent())
  conn.send(cpu_percent)
  print('written ' + cpu_percent)
  conn.close()
  print('client disconnected')
