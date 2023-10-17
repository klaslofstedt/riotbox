#!/bin/bash

# Check if the port is being used by a process and kill it
source ../../../.env
PID=$(lsof -t $THING_SERIAL_PORT)
if [ $? -eq 0 ]; then
    kill $PID
    sleep 1
fi