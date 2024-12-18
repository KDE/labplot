#!/usr/bin/python3

import socket, psutil, time

HOST = 'localhost'
PORT = 1027
ADDR = (HOST,PORT)
serv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
serv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

serv.bind(ADDR)
serv.listen(1)

print ('listening ...')
conn, addr = serv.accept()

while True:

  #message = str(psutil.cpu_percent())
  # multiple values
  message = str(psutil.cpu_percent()) + " " + str(psutil.boot_time()) + " " + str(psutil.cpu_count()) + "\n"
  try:
    conn.send(message.encode())
    print('written ' + message)
    time.sleep(0.1)
  except:
    conn.close()
    print('client disconnected')
    print ('listening ...')
    conn, addr = serv.accept()
    print('client connected ... ', addr)
