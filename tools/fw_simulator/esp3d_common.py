#!/usr/bin/python
#Common ESP3D snippets
import time

class bcolors:
    COL_PURPLE = '\033[95m'
    COL_BLUE = '\033[94m'
    COL_CYAN = '\033[96m'
    COL_GREEN = '\033[92m'
    COL_ORANGE = '\033[93m'
    COL_RED = '\033[91m'
    END_COL = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


def current_milli_time():
    return round(time.time() * 1000)

def send_echo(ser, msg):
    ser.write((msg + "\n").encode('utf-8'))
    print(bcolors.END_COL + msg + bcolors.END_COL)