from socket import socket, AF_INET, SOCK_DGRAM
from contextlib import closing
import datetime

HOST      = '192.168.4.1'
raspiPORT = 5001
m5PORT    = 5000
m5ADDRESS = "192.168.4.100"


s = socket(AF_INET, SOCK_DGRAM)
s.settimeout(1.0)
s.bind((HOST, raspiPORT))
s.close()
with closing(s):
    while True:
        msg = input("> ")
        start = datetime.datetime.now()
        s.sendto(msg.encode(),(m5ADDRESS,m5PORT))
        try:
            msg, address = s.recvfrom(4096)
            msg = msg.decode()
#            msg = s.recv(4096)
        except:
            msg = "timeout"
            address = "none"
#            print("exceptmes")
        print(f"message: {msg}\nfrom:{address}")
        print(datetime.datetime.now() - start)
s.close()
