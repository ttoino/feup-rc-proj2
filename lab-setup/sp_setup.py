#!/usr/bin/env python 
from __future__ import print_function
import serial
import io
import sys

if len(sys.argv) < 3:
	print("Usage: ", sys.argv[0], " {Serial Port device path} {command file path}")
	exit()

addr = sys.argv[1]   # serial port to read data from
baud = 115200	     # baud rate for serial port
fname = sys.argv[2]	 # file to read the commands from

with serial.Serial(addr, 9600) as pt, open(fname, 'r') as cmdsfile:
	serialPortInterface = io.TextIOWrapper(io.BufferedRWPair(pt, pt, 1),
										   encoding='ascii', errors='ignore',
										   newline='', line_buffering=True)
	serialPortInterface.readline()
	while True:
		cmd = cmdsfile.readline() + '\n'
		if cmd == '':
			break
		print(cmd, sep='')
		serialPortInterface.write(cmd)
	print("Finished reading lines.")