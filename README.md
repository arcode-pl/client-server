# client-server
This is socket client-server communication written in c.

#Installation (Ubuntu 20.04)

sudo apt-get install build-essential
sudo apt-get install libncurses5-dev

make.sh

#Run locally.

Server handling upto 10 client instances. 
To see ping-pong from client to server open in separate terminal window.

#Server: 
./server 
- close server by press ESC to propertly close all connections

#Clients (up to 10)
./client 127.0.0.1
