# client-server
This is client-server sockets communication written in C

#Installation (Ubuntu 20.04)
- sudo apt-get install build-essential
- sudo apt-get install libncurses5-dev
- make.sh

#Run locally.
- to see ping-pong from client to server open each app in separate terminal window.

#Server: 
- ./server for run
- close server by press ESC to propertly close all connections
- server handling upto 10 client instances

#Clients (up to 10)
- ./client 127.0.0.1 or ./run_client.sh for run
- close app by press ESC to propertly close socket

PS: Tested on 10 clients and one server without issues.
