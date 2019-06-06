import socket
import sys
import time

TCP_PORT = 2345

BUFFER_SIZE = 1024

def main():

    target_ip_addr = sys.argv[1]
    message = " ".join(x for x in sys.argv[2:])

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((target_ip_addr, TCP_PORT))
    s.send(message.encode())

    waiting = True
    while (waiting):
        data = s.recv(BUFFER_SIZE)
        if data:
            waiting = False
    s.close()

    data = data.decode()[:-4]
    print(data)

if __name__ == "__main__":
    main()
