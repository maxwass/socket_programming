#!/bin/bash

clear   # clear terminal window

echo "Startup script ... "

echo "Compiling server and client"
gcc client.c -o client
gcc server.c -o server

echo "Executing server and client ..."
./server 3490 & ./client localhost 3490 message_from_client_____1_ & ./client localhost 3490 message_from_client_____2_ &  ./client localhost 3490 message_from_client_____3_
