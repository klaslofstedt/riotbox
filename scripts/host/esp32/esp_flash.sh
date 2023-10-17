#!/bin/bash

#SERIAL_PORT="/dev/tty.usbserial-0001"
BAUD_RATE="460800"
BUILD_DIR="../../../app/thing/build"
source ../../../.env
./esp_reset_port.sh
# Wait 
sleep 1

# Flash FW to esp32
python3 -m esptool -p $THING_SERIAL_PORT -b $BAUD_RATE --before default_reset --after hard_reset --chip esp32 write_flash --flash_mode dio --flash_size 4MB --flash_freq 40m \
    0x1000 $BUILD_DIR/bootloader/bootloader.bin \
    0x8000 $BUILD_DIR/partition_table/partition-table.bin \
    0xd000 $BUILD_DIR/ota_data_initial.bin \
    0x10000 $BUILD_DIR/pre_encrypted_ota.bin
