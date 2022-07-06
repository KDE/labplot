#!/usr/bin/python3

import socket, psutil

HOST = 'localhost'
PORT = 1027
ADDR = (HOST,PORT)
serv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
serv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

serv.bind(ADDR)
serv.listen(1)

print ('listening ...')

while True:
  conn, addr = serv.accept()
  print('client connected ... ', addr)
  #message = str(psutil.cpu_percent())
  # more values
  message = str(psutil.cpu_percent()) + " " + str(psutil.boot_time()) + " " + str(psutil.cpu_count())
  conn.send(message.encode())
  print('written ' + message)
  conn.close()
  print('client disconnected')
