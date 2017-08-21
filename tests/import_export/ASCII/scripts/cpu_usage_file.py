import socket
import psutil
import time


while True:
    cpu_percents = str(psutil.cpu_percent(interval=0.5, percpu=True))
    line = str(cpu_percents)[1:-1]
    print line
    
    datafile = open("cpuData.txt","a")
    datafile.write(line + "\n")
    datafile.close()
