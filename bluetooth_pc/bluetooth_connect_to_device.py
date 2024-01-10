import bluetooth
import time
from address_devices import *
import sys

def mia_print(output, end="\n"):
    # with open('output.txt', 'w') as sys.stdout:
    print(output)

def main():
    global continua_esecuzione
    try:
        # with open('input.txt','r') as sys.stdin:
            # mia_print(".", end="")
            # t = int(input())
        message = input("Invia:")
        if (message=="STOP"):
            continua_esecuzione = False
        sock.send(message)
        # time.sleep(20)
    except EOFError:
        return
   
    
   
    


sock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
port = 1  # SPP profile on ESP32
sock.connect((address_devices.NFC_ESP32_BT_ADDRESS, port))
mia_print("Connesso")
seconds_keep_alive = 10
start_time = time.time()
continua_esecuzione = True
try:
    while continua_esecuzione:
        main()
        data = sock.recv(1024)
        print("data:" + str(data))
        current_time = time.time()
        if current_time - start_time >= seconds_keep_alive:
            sock.send("keep_alive")
            start_time = current_time
except KeyboardInterrupt:
    print()

print("Fine")
sock.close()