import bluetooth

# Replace with the Bluetooth address of your ESP32
esp32_address = '24:62:AB:F9:21:28'

def read_data():
    while True:
        try:
            data = client_socket.recv(1024).decode('utf-8')
            if data:
                print(f"Received from ESP32: {data}")
        except bluetooth.btcommon.BluetoothError:
            # Handle disconnection or errors
            print("Connection lost.")
            break

def write_data(data_to_send):
    client_socket.send(data_to_send.encode('utf-8'))
    print(f"Sent to ESP32: {data_to_send}")

if __name__ == "__main__":
    # Connect to ESP32 over Bluetooth
    client_socket = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
    client_socket.connect((esp32_address, 1))  # 1 is the channel for RFCOMM

    # Example of reading data in a separate thread
    import threading

    # Start the read_data function in a separate thread
    read_thread = threading.Thread(target=read_data)
    read_thread.start()

    # Example of writing data
    while True:
        user_input = input("Enter data to send to ESP32 (or 'exit' to quit): ")
        if user_input.lower() == 'exit':
            break
        write_data(user_input)

    # Close the Bluetooth connection when done
    client_socket.close()

