import bluetooth
import time
from address_devices import *
import sys
import threading

def mia_print(output, end="\n"):
    # with open('output.txt', 'w') as sys.stdout:
    print(output)

def chiedi_e_manda_dati():
    global continua_esecuzione
    try:
        # with open('input.txt','r') as sys.stdin:
            # mia_print(".", end="")
            # t = int(input())
        message = input("Invia:")
        if (message=="STOP"):
            continua_esecuzione = False
        sock.send(message)
        print("Mando: " + message)
        # time.sleep(20)
    except EOFError:
        print("EOF")
   
   
    


sock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
port = 1  # SPP profile on ESP32
sock.connect((address_devices.NFC_ESP32_BT_ADDRESS, port))
sock.setblocking(False)
mia_print("Connesso")



seconds_keep_alive = 10
start_time = time.time()
continua_esecuzione = True
try:
    while continua_esecuzione:
        # chiedi_e_manda_dati()
        t1 = time.time()
        data = sock.recv(30)
        t2 = time.time()

        print("data:" + str(data) + "\tTempo: " + str(t2-t1))
        current_time = time.time()
        if current_time - start_time >= seconds_keep_alive:
            sock.send("keep_alive")
            start_time = current_time
except KeyboardInterrupt:
    print()

print("Fine")
sock.close()