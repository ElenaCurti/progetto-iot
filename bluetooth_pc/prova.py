
# Per mandare dati:
import time




def send_data(data):
    
    
    sock.send(data)
    

    while True:
        message = input("Enter message to send to ESP32: ")
        send_data(message)
        time.sleep(1)


