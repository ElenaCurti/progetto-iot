import bluetooth
import threading
import time

def send_hello_periodically(client_socket):
    while True:
        time.sleep(5)
        try:
            client_socket.send("hello".encode('utf-8'))
        except Exception as e:
            print(f"Error sending 'hello': {e}")

def start_bluetooth_server():
    server_socket = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
    server_socket.bind(("", bluetooth.PORT_ANY))
    server_socket.listen(1)

    port = server_socket.getsockname()[1]

    print(f"Waiting for connection on RFCOMM channel {port}")

    bluetooth.advertise_service(
        server_socket,
        "BluetoothServer",
        service_id=bluetooth.SERIAL_PORT_CLASS,
        service_classes=[bluetooth.SERIAL_PORT_CLASS],
        profiles=[bluetooth.SERIAL_PORT_PROFILE],
    )

    client_socket, client_info = server_socket.accept()
    print(f"Accepted connection from {client_info}")

    try:
        while True:
            data = client_socket.recv(1024)
            if not data:
                break

            print(f"Received: {data.decode('utf-8')}")

            # Send "hello" every 5 seconds
            threading.Thread(target=send_hello_periodically, args=(client_socket,), daemon=True).start()


    except KeyboardInterrupt:
        pass

    print("Closing connection")
    client_socket.close()
    server_socket.close()
    print("Server closed")

if __name__ == "__main__":
    start_bluetooth_server()
