#!/usr/bin/python3

import pyudev
import os

context = pyudev.Context()
monitor = pyudev.Monitor.from_netlink(context)
monitor.filter_by(subsystem='usb')

for device in iter(monitor.poll, None):
	if device.action == 'add':
		print('{} connected'.format(device))
		os.system("/usr/local/bin/usb-flash.sh")

