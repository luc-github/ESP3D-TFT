#!/usr/bin/python
import serial
import serial.tools.list_ports
import time
import re
import random


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


X_current = 0.0
Y_current = 0.0
Z_current = 0.0
mode_absolute = True

temperatures = {"E0": {"value": 0.0, "target": 0.0, "lastTime": -1, "heatspeed": 0.6, "coolspeed": 0.8, "variation": 0.5},
                "B": {"value": 0.0, "target": 0.0, "lastTime": -1, "heatspeed": 0.2, "coolspeed": 0.8, "variation": 0.5}}


def current_milli_time():
    return round(time.time() * 1000)


def updateTemperatures(entry, timestp):
    global temperatures
    roomtemp = 20.0
    v = random.random()*5
    target = temperatures[entry]["target"]
    if target == 0:
        target = roomtemp
    if (temperatures[entry]["value"] == 0):
        temperatures[entry]["value"] = roomtemp + v / 2
    if (temperatures[entry]["lastTime"] == -1):
        temperatures[entry]["lastTime"] = timestp
    if temperatures[entry]["value"] + 5 < target:
        temperatures[entry]["value"] = temperatures[entry]["value"] + \
            (temperatures[entry]["heatspeed"] *
             (timestp - temperatures[entry]["lastTime"])) / 1000
    elif temperatures[entry]["value"] - 5 > target:
        temperatures[entry]["value"] = temperatures[entry]["value"] - \
            (temperatures[entry]["coolspeed"] *
             (timestp - temperatures[entry]["lastTime"])) / 1000
    elif target - 2 < temperatures[entry]["value"] and temperatures[entry]["value"] < target + 2:
        temperatures[entry]["value"] = target + \
            temperatures[entry]["variation"] * (random.random() - 0.5)
    elif temperatures[entry]["value"] < target:
        temperatures[entry]["value"] = temperatures[entry]["value"] + \
            ((temperatures[entry]["heatspeed"]/3) *
             (timestp - temperatures[entry]["lastTime"])) / 1000
    else:
        temperatures[entry]["value"] = temperatures[entry]["value"] - \
            ((temperatures[entry]["coolspeed"]/3) *
             (timestp - temperatures[entry]["lastTime"])) / 1000

    temperatures[entry]["lastTime"] = timestp


def generateTemperatureResponse(withok):
    global temperatures
    response = " "
    if (withok):
        response = "ok "
    response += "T:" + "{:.2f}".format(temperatures["E0"]["value"]) + " /" + "{:.2f}".format(temperatures["E0"]["target"]) + " B:" + "{:.2f}".format(
        temperatures["B"]["value"]) + " /" + "{:.2f}".format(temperatures["B"]["target"]) + " @:127 B@:0"
    return response


def send_echo(ser, msg):
    ser.write((msg + "\n").encode('utf-8'))
    print(bcolors.END_COL + msg + bcolors.END_COL)


def display_busy(ser, nb):
    v = nb
    while (v > 0):
        send_echo(ser, "echo:busy: processing")
        time.sleep(1)
        v = v - 1


def processLine(line, ser):
    time.sleep(0.01)
    global X_current, Y_current, Z_current, mode_absolute
    global temperatures
    X_val = ""
    Y_val = ""
    Z_val = ""
    if (line.startswith("G91")):
        mode_absolute = False
        return "ok"
    if (line.startswith("G90")):
        mode_absolute = True
        return "ok"
    if (line.startswith("G1") or line.startswith("G0")):
        # extract X
        X = re.findall('X[+]*[-]*\d+[\.]*\d*', line)
        if (len(X) > 0):
            X_val = X[0][1:]
        # extract Y
        Y = re.findall('Y[+]*[-]*\d+[\.]*\d*', line)
        if (len(Y) > 0):
            Y_val = Y[0][1:]
        # extract Z
        Z = re.findall('Z[+]*[-]*\d+[\.]*\d*', line)
        if (len(Z) > 0):
            Z_val = Z[0][1:]
        if (mode_absolute):
            if (X_val != ""):
                X_current = float(X_val)
            if (Y_val != ""):
                Y_current = float(Y_val)
            if (Z_val != ""):
                Z_current = float(Z_val)
            return "ok"
        else:
            if (X_val != ""):
                X_current += float(X_val)
            if (Y_val != ""):
                Y_current += float(Y_val)
            if (Z_val != ""):
                Z_current += float(Z_val)
            return "ok"
    if (line.startswith("G28")):
        # let simulate the output processing
        send_echo(ser, "echo:busy: processing")
        time.sleep(1)
        send_echo(ser, "echo:busy: processing")
        time.sleep(1)
        send_echo(ser, "echo:busy: processing")
        time.sleep(1)
        if (line.find("X") != -1):
            X_current = 0.00
        if (line.find("Y") != -1):
            Y_current = 0.00
        if (line.find("Z") != -1):
            Z_current = 0.00
        if (line == "G28"):
            X_current = 0.00
            Y_current = 0.00
            Z_current = 0.00
        return "ok"
    if (line.startswith("M104")):
        targettemp = re.findall('S\d+[\.]*\d*', line)
        if (len(targettemp) > 0):
            temperatures["E0"]["target"] = float(targettemp[0][1:])
        return "ok"
    if (line.startswith("M109")):
        targettemp = re.findall('[SR]\d+[\.]*\d*', line)
        if (len(targettemp) > 0):
            temperatures["E0"]["target"] = float(targettemp[0][1:])
            target = 20.0
            if (temperatures["E0"]["target"] != 0):
                target = temperatures["E0"]["target"]
            while (temperatures["E0"]["value"] < target-2 or temperatures["E0"]["value"] > target+2):
                send_echo(ser, "echo:busy: processing")
                time.sleep(1)
                updateTemperatures("E0", current_milli_time())
                updateTemperatures("B", current_milli_time())
                val = generateTemperatureResponse(False)
                send_echo(ser, val)
                time.sleep(1)
                updateTemperatures("E0", current_milli_time())
                updateTemperatures("B", current_milli_time())
                val = generateTemperatureResponse(False)
                send_echo(ser, val)
            return ""
    if (line.startswith("M190")):
        targettemp = re.findall('[SR]\d+[\.]*\d*', line)
        if (len(targettemp) > 0):
            temperatures["B"]["target"] = float(targettemp[0][1:])
            target = 20.0
            if (temperatures["E0"]["target"] != 0):
                target = temperatures["E0"]["target"]
            while (temperatures["B"]["value"] < target-2 or temperatures["B"]["value"] > target+2):
                send_echo(ser, "echo:busy: processing")
                time.sleep(1)
                updateTemperatures("E0", current_milli_time())
                updateTemperatures("B", current_milli_time())
                val = generateTemperatureResponse(False)
                send_echo(ser, val)
                time.sleep(1)
                updateTemperatures("E0", current_milli_time())
                updateTemperatures("B", current_milli_time())
                val = generateTemperatureResponse(False)
                send_echo(ser, val)
            return ""
    if (line.startswith("M140")):
        targettemp = re.findall('S\d+[\.]*\d*', line)
        if (len(targettemp) > 0):
            temperatures["B"]["target"] = float(targettemp[0][1:])
        return "ok"
    if (line.startswith("M105")):
        updateTemperatures("E0", current_milli_time())
        updateTemperatures("B", current_milli_time())
        val = generateTemperatureResponse(True)
        return val
    if (line.startswith("M114")):
        val = "X:" + "{:.2f}".format(X_current) + " Y:" + "{:.2f}".format(
            Y_current)+" Z:" + "{:.2f}".format(Z_current)+" E:0.00 Count X:0 Y:0 Z:0"
        return val
    if (line.startswith("M220")):
        val = re.findall('S\d+', line)
        if (len(val) > 0):
            F_R = val[0][1:]
            return "FR:"+F_R+"%\nok"
    if (line.startswith("M106")):
        return line+"\nok"
    if (line.startswith("M107")):
        val = re.findall('P\d+', line)
        if (len(val) > 0):
            return "M106 P" + val[0][1:] + " S0\nok"
        else:
            return "M106 P0 S0\nok"
    if (line.startswith("G29 V4")):
        send_echo(ser, " G29 Auto Bed Leveling")
        display_busy(ser, 3)
        send_echo(ser, "Bed X: 50.000 Y: 50.000 Z: 0.000")
        display_busy(ser, 3)
        send_echo(ser, "Bed X: 133.000 Y: 50.000 Z: 0.016")
        display_busy(ser, 3)
        send_echo(ser, "Bed X: 216.000 Y: 50.000 Z: -0.013")
        display_busy(ser, 3)
        send_echo(ser, "Bed X: 299.000 Y: 50.000 Z: -0.051")
        display_busy(ser, 3)
        send_echo(ser, "Bed X: 299.000 Y: 133.000 Z: -0.005")
        display_busy(ser, 3)
        send_echo(ser, "Bed X: 216.000 Y: 133.000 Z: -0.041")
        display_busy(ser, 3)
        send_echo(ser, "Bed X: 133.000 Y: 133.000 Z: -0.031")
        display_busy(ser, 3)
        send_echo(ser, "Bed X: 50.000 Y: 133.000 Z: -0.036")
        display_busy(ser, 3)
        send_echo(ser, "Bed X: 50.000 Y: 216.000 Z: -0.050")
        display_busy(ser, 3)
        send_echo(ser, "Bed X: 133.000 Y: 216.000 Z: 0.055")
        display_busy(ser, 3)
        send_echo(ser, "Bed X: 216.000 Y: 216.000 Z: 0.051")
        display_busy(ser, 3)
        send_echo(ser, "Bed X: 299.000 Y: 216.000 Z: 0.026")
        display_busy(ser, 3)
        send_echo(ser, "Bed X: 299.000 Y: 299.000 Z: -0.018")
        display_busy(ser, 3)
        send_echo(ser, "Bed X: 216.000 Y: 299.000 Z: -0.064")
        display_busy(ser, 3)
        send_echo(ser, "Bed X: 133.000 Y: 299.000 Z: -0.036")
        display_busy(ser, 3)
        send_echo(ser, "Bed X: 50.000 Y: 299.000 Z: -0.046")
        display_busy(ser, 3)
        send_echo(ser, "Bilinear Leveling Grid:")
        send_echo(ser, "      0      1      2      3")
        send_echo(ser, " 0 +0.0000 +0.0162 -0.0125 -0.0512")
        send_echo(ser, " 1 -0.0363 -0.0313 -0.0412 -0.0050")
        send_echo(ser, " 2 -0.0500 +0.0550 +0.0512 +0.0262")
        send_echo(ser, " 3 -0.0463 -0.0363 -0.0638 -0.0175")
        return "ok"

    return "ok"


def main():
    ports = serial.tools.list_ports.comports()
    portTFT = ""
    print(bcolors.COL_GREEN+"Serial ports:"+bcolors.END_COL)
    for port, desc, hwid in sorted(ports):
        print(bcolors.COL_GREEN+"{}: {} ".format(port, desc)+bcolors.END_COL)
        desc.capitalize()
        if (desc.find("SERIAL") != -1):
            portTFT = port
            print(bcolors.COL_GREEN +
                  "Found " + portTFT + " for TFT"+bcolors.END_COL)
            break
    print(bcolors.COL_GREEN+"Open port " + str(port)+bcolors.END_COL)
    if (portTFT == ""):
        print(bcolors.COL_RED+"No serial port found"+bcolors.END_COL)
        exit(0)
    ser = serial.Serial(portTFT, 115200)

    starttime = current_milli_time()
    # loop forever, just unplug the port to stop the program or do ctrl-c
    while True:
        try:
            if ser.in_waiting:
                line = ser.readline().decode('utf-8').strip()
                print(bcolors.COL_BLUE+line+bcolors.END_COL)
                response = processLine(line, ser)
                if (response != ""):
                    send_echo(ser, response)
        except:
            print(bcolors.COL_GREEN+"End of program"+bcolors.END_COL)
            exit(0)


# call main function
main()
