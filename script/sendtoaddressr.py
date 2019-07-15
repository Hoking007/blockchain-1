#!/usr/bin/python

# simulate a large number of transactions in a short time

import os
import time

address = input("Please enter dest address: ")
amount = input("Please enter amount(recommend 0.1): ")

while True:
    os.system("bitcoin-cli sendtoaddress address amount")
    time.sleep(0.1) # Ctrl+C to exit
