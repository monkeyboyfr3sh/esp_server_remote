import socket
import threading
import time

import serial_tcp_rx as s_rx
import serial_tcp_tx as s_tx

HOST = "127.0.0.1"  # The server's hostname or IP address
PORT = 5555  # The port used by the server

if __name__ == "__main__":
    print("main thread enters")
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        
        # Connect to host
        print("Connecting to server... ",end="")
        s.connect((HOST, PORT))
        print("Connected")
        
        # Fork tx thread
        print("Forking tx thread")
        tx_thread = threading.Thread(target=s_tx.tx_serial_thread,args=(s,))
        tx_thread.start()

        # Fork rx thread
        print("Forking rx thread")
        rx_thread = threading.Thread(target=s_rx.rx_serial_thread,args=(s,))
        rx_thread.start()

        while(1):
            time.sleep(1)
