#!/usr/bin/python
import serial
import serial.tools.list_ports
import time
import re

E0_current = 0.0
E0_target = 0.0
B_current = 0.0
B_target = 0.0
X_current = 10.0
Y_current = 0.0
Z_current = 0.0
F_R = "0"
mode_absolute = True

def send_echo(ser, msg):
     ser.write((msg + "\n").encode('utf-8'))
     print(msg)

def processLine(line,ser):
     global X_current, Y_current, Z_current,mode_absolute,E0_current,E0_target,B_current,B_target
     X_val = ""
     Y_val = ""
     Z_val = ""
     if(line.startswith("G91")):
          mode_absolute = False
          return "ok"
     if(line.startswith("G90")):
          mode_absolute = True
          return "ok"
     if(line.startswith("G1") or line.startswith("G0")):
          #extract X
          X = re.findall('X[+]*[-]*\d+[\.]*\d*', line)

          if(len(X) > 0):
               X_val = X[0][1:]
          #extract Y
          Y = re.findall('Y[+]*[-]*\d+[\.]*\d*', line)
          if(len(Y) > 0):
               Y_val = Y[0][1:]
          #extract Z
          Z = re.findall('Z[+]*[-]*\d+[\.]*\d*', line)
          if(len(Z) > 0):
               Z_val = Z[0][1:]
          if(mode_absolute):
               if (X_val!=""):
                    X_current =  float(X_val)
               if (Y_val!=""):
                    Y_current = float(Y_val)
               if (Z_val!=""):
                    Z_current = float(Z_val)
               return "ok"
          else:
               if (X_val!=""):
                    X_current +=  float(X_val)
               if (Y_val!=""):
                    Y_current += float(Y_val)
               if (Z_val!=""):
                    Z_current += float(Z_val)
               return "ok"
     if(line.startswith("G28")):
          #let simulate the output processing
          send_echo(ser, "echo:busy: processing")
          time.sleep(1)
          send_echo(ser, "echo:busy: processing")
          time.sleep(1)
          send_echo(ser, "echo:busy: processing")
          time.sleep(1)
          if (line.find("X") != -1):
               X_current = 0.00
          if (line.find("Y")!= -1):
               Y_current = 0.00
          if (line.find("Z")!= -1):
               Z_current = 0.00
          if (line== "G28"):
               X_current = 0.00
               Y_current = 0.00
               Z_current = 0.00
          return "ok"
     if(line.startswith("M105")):
          return "ok T:25.00 /120.00 B:25.00 /0.00 @:127 B@:0"
     if (line.startswith("M114")):
          val = "X:"+ "{:.2f}".format(X_current)+ " Y:"+ "{:.2f}".format(Y_current)+" Z:"+ "{:.2f}".format(Z_current)+" E:0.00 Count X:0 Y:0 Z:0"
          return val
     if (line.startswith("M220")):
           val = re.findall('S\d+', line)
           if(len(val) > 0):
               F_R = val[0][1:]
               return "FR:"+F_R+"%\nok"
     if (line.startswith("M106")):
           return line+"\nok"
     return "ok"


def main():
    ports = serial.tools.list_ports.comports()
    portTFT=""
    print("Serial ports:")
    for port, desc, hwid in sorted(ports):
            print("{}: {} ".format(port, desc))
            desc.capitalize()
            if (desc.find("SERIAL")!= -1):
                portTFT = port
                print("Found " + portTFT)
                break
    print ("Open port " + str(port))
    if (portTFT == ""):
         print("No serial port found")
         exit(0)
    ser = serial.Serial(portTFT, 115200)

    starttime = time.time()
    #loop forever, just unplug the port to stop the program
    while True:
        #try:
        if ser.in_waiting:
            line = ser.readline().decode('utf-8').strip()
            print(line)
            response = processLine(line, ser)
            send_echo(ser, response)
        #except :
        #     print("End of program")
        #     exit(0)

#call main function        
main()


