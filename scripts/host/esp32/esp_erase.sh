#!/bin/bash

BAUD_RATE="460800"
source ../../../.env
./esp_reset_port.sh
wait 
sleep 1

#python3 -m esptool -p $THING_SERIAL_PORT -b $BAUD_RATE --before default_reset --after hard_reset --chip esp32 erase_flash
python3 -m esptool -p $THING_SERIAL_PORT -b $BAUD_RATE erase_flash
