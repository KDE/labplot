#!/usr/bin/python

import time
from scipy.stats import norm
import numpy as np

LIVE_DATA = True
LIVE_DATA_DEBUG = True

fileName = "out.txt" # name of the output file
sleepInterval = 1 # sleep interval in seconds
itersTotal = 100000 # total number of iterations to compute
iters = 1000 # number of iterations done before the results are written out to the file
pathes = 2 # Number of pathes to compute
delta = 0.25
dt = 0.1


# trajectories
x = np.zeros(pathes)

# Iterate to compute the steps of the Brownian motion.
i = 0
out = ""
for k in range(itersTotal):
	if out != "":
		out += "\n"

	out += str(k*dt)
	for p in range(pathes):
		x[p] += norm.rvs(scale=delta**2*dt)
		out += "\t" + str(x[p])

	i += 1

	if LIVE_DATA and i >= iters:
		if LIVE_DATA_DEBUG:
			print (out)
		# generate header
		outh = "t"
		for p in range(pathes):
			outh += "\tx" + str(p+1)

		datafile = open(fileName, "w")

		datafile.write(outh)
		datafile.write('\n')
		datafile.write(out)
		datafile.close()

		out = ""
		i = 0
		time.sleep(sleepInterval)

if LIVE_DATA and out != "":
	datafile = open(fileName, "a")
	datafile.write(out)
	datafile.close()
	if LIVE_DATA_DEBUG:
		print (out)

if not LIVE_DATA:
	datafile = open(fileName, "a")
	datafile.write(out)
