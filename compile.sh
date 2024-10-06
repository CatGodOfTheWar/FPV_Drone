#!/bin/bash

rm -f main DSHOT.s
gcc -O2 -S -o DSHOT.s DSHOT.c
g++ -O2 -Wall -o main main.cpp DSHOT.c  -lpthread -lpigpio

echo "Compile complete!"