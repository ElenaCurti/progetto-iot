import threading
import bluetooth
from address_devices import *
import time
import sys
from inputimeout import inputimeout, TimeoutOccurred
import os, signal

# ---------------------------------------------------------------------------------------------

# Le prossime due variabili servono per controllare che la connessione BT venga 
# chiusa (per un qualche motivo) dalla ESP. Lo script terminera' al massimo dopo
# SECONDS_KEEP_ALIVE+SECONDS_TIMEOUT_INPUT secondi dalla chiusura della connessione.
# Per chiudere la connessione dallo script, invece, basta dare STOP come input.

# Ogni SECONDS_KEEP_ALIVE viene mandato un messaggio di "keep_alive" alla board
SECONDS_KEEP_ALIVE = 5

# Ogni SECONDS_TIMEOUT_INPUT viene controllato che l'utente abbia inserito 
# qualcosa in input, per evitare di aspettare in eterno
SECONDS_TIMEOUT_INPUT = 10


# Massima dimensione dei messaggi in input (in bytes)
MAX_DIM_MESSAGES = 1024 # TODO controlla che sia cosi'

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
        print_debug("Connesso a CAM")
    elif sys.argv[1] == "--nfc":
        sock.connect((address_devices.NFC_ESP32_BT_ADDRESS, port))
        print_debug("Connesso a NFC")
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

# Define a function for reading data
def read_data(mio_sock):
    global continua_esecuzione
    try: 
        while continua_esecuzione:
            data = mio_sock.recv(MAX_DIM_MESSAGES)  
            if len(data) > 0:
                string_data = data.decode('utf-8')
                print(string_data, flush=True)
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
    start_time = time.time()
    try:
        while continua_esecuzione:
            current_time = time.time()
            if current_time - start_time >= SECONDS_KEEP_ALIVE:
                mio_sock.send(message_keep_alive)
                start_time = current_time
            time.sleep(2)
    except OSError:
        fine(4) 
        return

# Create threads for reading and sending data
read_thread = threading.Thread(target=read_data, args=(sock,))
send_thread = threading.Thread(target=send_data, args=(sock,))
keep_alive_thread = threading.Thread(target=keep_alive, args=(sock,))

# Start the threads
read_thread.start()
keep_alive_thread.start()
send_thread.start()
