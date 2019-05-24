import serial
import sys

if len(sys.argv) < 2:
    print("Path for serial device needed")
    exit(1)

serialport = serial.Serial(sys.argv[1], 115200, timeout=0.5)

excluded = []
excluded.append("IPMC 20:unknown failure")
excluded.append("event message delivery failed")

while True:   
    line = serialport.readline()
    if len(line) > 0:
        line = line.rstrip()
        success = 0
        while success == 0 :
            
            try:
                decoded = line.decode()
                success = 1
            except:
                line = line[1:-1]

        printit = True
        for e in excluded:
            if e in decoded:
                printit = False
        if printit is True:
            print(decoded)
