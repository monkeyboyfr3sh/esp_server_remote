import socket
import threading
import time



def rx_serial_thread(socket):
    print("RX serial thread enters")
    
    # Thread now loops
    string_buff = ""
    while(1):
        # Receive response
        rx_data = socket.recv(1024)
        rx_string = (str)(rx_data.decode('ascii'))
        
        # Get data
        string_list = rx_string.split('\n')
        string_buff += string_list[0]
        
        # Check for new line break
        if(len(string_list)>1):
            string_buff.strip('\n')
            print("got this" + string_buff)
            string_buff = ""