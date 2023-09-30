#!/usr/bin/python
import serial
import serial.tools.list_ports
import esp3d_common as common
import marlin 

fw = marlin

def main():
    ports = serial.tools.list_ports.comports()
    portTFT = ""
    print(common.bcolors.COL_GREEN+"Serial ports:"+common.bcolors.END_COL)
    for port, desc, hwid in sorted(ports):
        print(common.bcolors.COL_GREEN+"{}: {} ".format(port, desc)+common.bcolors.END_COL)
        desc.capitalize()
        if (desc.find("SERIAL") != -1):
            portTFT = port
            print(common.bcolors.COL_GREEN +
                  "Found " + portTFT + " for TFT"+common.bcolors.END_COL)
            break
    print(common.bcolors.COL_GREEN+"Open port " + str(port)+common.bcolors.END_COL)
    if (portTFT == ""):
        print(common.bcolors.COL_RED+"No serial port found"+common.bcolors.END_COL)
        exit(0)
    ser = serial.Serial(portTFT, 115200)

    starttime = common.current_milli_time()
    # loop forever, just unplug the port to stop the program or do ctrl-c
    while True:
        try:
            if ser.in_waiting:
                line = ser.readline().decode('utf-8').strip()
                print(common.bcolors.COL_BLUE+line+common.bcolors.END_COL)
                #ignore log lines from TFT
                if not line.startswith("["):
                    response = fw.processLine(line, ser)
                    if (response != ""):
                        common.send_echo(ser, response)
        except KeyboardInterrupt:
            print(common.bcolors.COL_GREEN+"End of program"+common.bcolors.END_COL)
            exit(0)


# call main function
main()
