#!/usr/bin/python

import socket, psutil, os, stat

ADDR = './local_socket'
serv = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

st = os.stat(ADDR)
if stat.S_ISSOCK(st.st_mode):
  print 'socket exists. Removing it'
  os.remove(ADDR)

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
