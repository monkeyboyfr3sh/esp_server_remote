import socket
import threading
import time

HOST = "127.0.0.1"  # The server's hostname or IP address
PORT = 5555  # The port used by the server

import select

def rx_serial_thread(socket):
    print("RX serial thread enters")
    while(1):
        # Receive response
        rx_data = socket.recv(1024)
        rx_string = rx_data.decode('ascii')
        print(rx_string,end="")

def tx_serial_thread(socket):
    print("TX serial thread enters")
    while(1):
        # Transmit data
        # print("Type your message:", end=' ')
        tx_string = input()
        tx_string = tx_string+"\x0A"
        tx_data = tx_string.encode()
        socket.sendall(tx_data)
        time.sleep(1)

if __name__ == "__main__":
    print("main thread enters")
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        
        # Connect to host
        print("Connecting to server... ",end="")
        s.connect((HOST, PORT))
        print("Connected")
        
        # Fork tx thread
        print("Forking tx thread")
        tx_thread = threading.Thread(target=tx_serial_thread,args=(s,))
        tx_thread.start()

        # Fork rx thread
        print("Forking rx thread")
        rx_thread = threading.Thread(target=rx_serial_thread,args=(s,))
        rx_thread.start()

        while(1):
            time.sleep(1)
