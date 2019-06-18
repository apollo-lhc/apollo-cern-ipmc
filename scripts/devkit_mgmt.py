import serial
import sys

if (len(sys.argv) < 2) and ("tty" not in sys.argv[1].lower()):
    print("Path for serial device needed")
    print("Usage: {0} <command>".format(sys.argv[0]))
    exit(1)

serialport = serial.Serial(sys.argv[1], 9600, timeout=0.5)


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
