import threading
import bluetooth
from address_devices import *
import time
import sys
from inputimeout import inputimeout, TimeoutOccurred    # Utili se dati da terminale, non funzionano con node-red
import os, signal
import subprocess as sp


# ---------------------------------------------------------------------------------------------


# Massima dimensione dei messaggi in input (in bytes)
MAX_DIM_MESSAGES = 2000000 

# ---------------------------------------------------------------------------------------------

def print_debug(*args, **kwargs):
    global DEBUG
    if DEBUG:
        print(args, kwargs)


port = 1  # SPP profile on ESP32
continua_esecuzione = True
message_keep_alive = "keep_alive"
DEBUG = len(sys.argv) >=3 and sys.argv[2] == "--debug"

# Create a Bluetooth socket
sock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
try:
    if sys.argv[1] == "--cam":
        sock.connect((address_devices.CAM_ESP32_BT_ADDRESS, port))
        print("Connesso (BT) a CAM")
    elif sys.argv[1] == "--nfc":
        sock.connect((address_devices.NFC_ESP32_BT_ADDRESS, port))
        print("Connesso (BT) a NFC")
    else:
        print("Parametro NON valido. Parametri validi: --cam oppure --nfc", file=sys.stderr)
        exit(2)
except OSError:
    print("Device irraggiungibile", file=sys.stderr)
    exit(3)



def fine(num):
    global sock, continua_esecuzione
    print_debug("Fine", num)
    continua_esecuzione = False
    sock.close()  

    # Devo "killare" tutto il processo perche' la funzione input in send_data mantiene il thread vivo
    os.system("taskkill /F /PID " + str(os.getpid()) + " 2> err > out ")    

def _recv_client_data(client_socket: bluetooth.BluetoothSocket):
    data = b''
    tmp_char = client_socket.recv(1)
    # c = 0
    while tmp_char != b'\x0A':  
        data += tmp_char
        tmp_char = client_socket.recv(1)
        # c+=1
        # print(c, end=" ")
    # print("FNIE")
    
    return data


# Define a function for reading data
def read_data(mio_sock):
    global continua_esecuzione
    try: 
        while continua_esecuzione:
            data = _recv_client_data(mio_sock)
            
            if len(data) > 0:
                # print(len(data))
                string_data = data.decode('utf-8')
                print(string_data, flush=True)
                # print(len(string_data))
                # print("--")
    except OSError:
        fine(1)
        return

# Define a function for sending data
def send_data(mio_sock):
    global continua_esecuzione
    while continua_esecuzione:
        # try: 
        data = input()
        if (data == "STOP"):
            fine(2)
        mio_sock.send(data)  # Send data to the socket
        # except KeyboardInterrupt:
            # continue
        
def keep_alive(mio_sock):    

    global continua_esecuzione, message_keep_alive

    import select
    while continua_esecuzione:
        ready_to_read, _, _ = select.select([mio_sock], [], [], 0.1)
        
        if ready_to_read:
            # The socket is still connected
            # print("Device is disconnected.")
            fine(5)
        time.sleep(10)
        # else:
            # The socket is not ready for reading, indicating a potential disconnection
            # print("Device is potentially disconnected.")
            # 


    
    # start_time = time.time()
    # try:
    #     while continua_esecuzione:
    #         current_time = time.time()
    #         if current_time - start_time >= SECONDS_KEEP_ALIVE:
    #             mio_sock.send(message_keep_alive)
    #             start_time = current_time
    #         time.sleep(2)
    # except OSError:
    #     fine(4) 
    #     return

# Create threads for reading and sending data
read_thread = threading.Thread(target=read_data, args=(sock,))
send_thread = threading.Thread(target=send_data, args=(sock,))
keep_alive_thread = threading.Thread(target=keep_alive, args=(sock,))

# Start the threads
read_thread.start()
keep_alive_thread.start()
send_thread.start()
