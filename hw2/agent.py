#! /usr/bin/env python3
# -*- coding: UTF-8 -*-
import sys
import signal
import psutil
import socket
import requests
import threading

def sig_handler(signum, frame):
    sys.exit(0)

signal.signal(signal.SIGINT, sig_handler)

HOSTNAME = socket.gethostname()
POST_URL = 'http://localhost:8888/info'

while True:
    cpu_load = psutil.cpu_percent(interval=0.25)
    try:
        requests.post(POST_URL, data={'hostname': HOSTNAME,
                                      'cpu_load': cpu_load,})
    except:
        print('connection error')
