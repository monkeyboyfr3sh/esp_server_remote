import socket
import threading
import time

inputs = [
    "button_pad_push -l 1 -t 1000 -e 25",
    "button_pad_push -l 0 -t 1500 -e 25",
]

def tx_serial_thread(socket):
    print("TX serial thread enters")
    count = 0
    while(1):
        # Get string
        tx_string = inputs[count]
        count += 1
        if (count==len(inputs)):
            count = 0
        # Transmit the string
        tx_string = tx_string+"\x0A"
        tx_data = tx_string.encode()
        socket.sendall(tx_data)

        time.sleep(.5)