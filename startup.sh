#!/bin/bash

clear   # clear terminal window

echo "Startup script ... "

echo "Compiling server and client"
gcc client.c -o client
gcc server.c -o server

echo "Executing server and client ..."
./server 3490 & ./client localhost 3490 _111111111_ & ./client localhost 3490 _22222222_ &  ./client localhost 3490 _333333333_
