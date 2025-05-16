#!/usr/bin/python3

import socket, psutil, time

HOST = 'localhost'
PORT = 1234
serv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
serv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

# bind host and port together
serv.bind((HOST,PORT))

# configure how many clients the server can listen simultaneosly
serv.listen(1)
print ('listening ...')

# accept new connections
conn, addr = serv.accept()
print("connection from: " + str(addr))

# send the current values for the server metrics
while True:
    cpu = psutil.cpu_percent();
    cpu_freq = psutil.cpu_freq().current;
    mem = psutil.virtual_memory()
    load = psutil.getloadavg()
    message = str(cpu) + " " + str(cpu_freq) + " " + str(mem[2]) + " " +  str(load[0]) + " " + str(load[1]) + " " + str(load[2]) + "\n"
    try:
        conn.send(message.encode())
        print('written ' + message)
        # wait 1s before sending the next message
        time.sleep(1)
    except:
        conn.close()
        print('client disconnected')
        print ('listening ...')
        conn, addr = serv.accept()
        print('client connected ... ', addr)
