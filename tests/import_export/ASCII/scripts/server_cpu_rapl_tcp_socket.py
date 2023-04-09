#!/usr/bin/python3

import socket, psutil, subprocess
import sys, errno

#HOST = '192.168.178.26'
HOST = 'localhost'
PORT = 1030
ADDR = (HOST,PORT)
serv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

serv.bind(ADDR)
serv.listen(1)

# download pinpoit from https://github.com/osmhpi/pinpoint
# use pinpoint -l to get the list of counters and aliases for them,
# adjust the command below - e.g. use CPU,RAM only if not counter is available for the GPU, etc.
pinpoint_cmd = "pinpoint -e CPU,GPU,RAM -i 200 -c sleep 0.1"

print ('listening ...')

while True:
  conn, addr = serv.accept()
  print('client connected ... ', addr)
  cpu_percent = str(psutil.cpu_percent())
  pinpoint_output = subprocess.check_output(pinpoint_cmd, shell=True).decode("utf-8")
  result = str(cpu_percent) + ',' + pinpoint_output
  
  try:
    conn.send(result.encode())
    print('written ' + result)
  except (BrokenPipeError, IOError):
    pass

  conn.close()
  print('client disconnected')
