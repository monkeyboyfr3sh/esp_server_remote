# esp-server-remote
This is a project based on [nopnop2002](https://github.com/nopnop2002)'s [esp-idf-ssh-client](https://github.com/nopnop2002/esp-idf-ssh-client) project. I'm making this into a device that sits on a desk and sends SSH commands at a button press. Development is happening on an [ESP32-LyraTD-MSC](https://www.espressif.com/en/products/devkits/esp32-lyratd-msc/overview) dev kit.

## esp-idf-ssh-client
ssh client for esp-idf.   
You can use the ssh API to execute remote command.   
This project use [this](https://github.com/libssh2/libssh2) ssh library.   

## Screen Shot
![ssh-client-1](https://user-images.githubusercontent.com/6020549/120056024-b1ffc200-c074-11eb-8507-1bb566b0cc7c.jpg)

## Reference
https://github.com/nopnop2002/esp-idf-scp-client

You can use scp and ssh to do heavy processing that esp32 alone cannot.  
- Copy file from esp32 to remote using scp-put.   
- Execute remote command using ssh-client.   
- The processing result is redirected to a file.   
- Copy file from remote to esp32 using scp-get.