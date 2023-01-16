import threading
import sys
import time
import string

LOG_FILENAME = "log.txt"
def rx_serial_thread():
    print("RX serial thread enters")

    while(1):
        for line in sys.stdin:
            f = open(LOG_FILENAME, "a")
            printable = set(string.printable)
            filter(lambda x: x in printable, line)
            f.write(line)
            f.close()

def tx_serial_thread():
    print("TX serial thread enters")
    while(1):
        # Transmit data
        # print("Type your message:", end=' ')
        tx_string = "string"
        print(tx_string+"\x0A")
        time.sleep(1)

if __name__ == "__main__":
    print("main thread enters")
    
    # Fork tx thread
    print("Forking tx thread")
    tx_thread = threading.Thread(target=tx_serial_thread)
    tx_thread.start()

    # Fork rx thread
    print("Forking rx thread")
    rx_thread = threading.Thread(target=rx_serial_thread)
    rx_thread.start()

    while(1):
        time.sleep(1)
