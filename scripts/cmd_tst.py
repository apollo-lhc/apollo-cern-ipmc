import socket
import sys
import time

TCP_PORT = 2345

BUFFER_SIZE = 1024

def main():


    target_ip_addr = sys.argv[1]
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((target_ip_addr, TCP_PORT))

    message = "get_gpio"
    s.send(message.encode())
    print(message)
    input("Press Enter to continue...")

    message = " "
    s.send(message.encode())
    print(message)
    input("Press Enter to continue...")

    message = "en_12v"
    s.send(message.encode())
    print(message)
    input("Press Enter to continue...")

    message = "\n"
    s.send(message.encode())
    print(message)
    input("Press Enter to continue...")

    waiting = True
    while (waiting):
        data = s.recv(BUFFER_SIZE)
        if data:
            waiting = False
    s.close()

    data = data.decode()[:-3]
    data = data.rstrip()
    print(data)

if __name__ == "__main__":
    main()
