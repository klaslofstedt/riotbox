#!/bin/bash

source ../../../.env
./esp_reset_port.sh
# Wait 
sleep 1

# Set the path to the secrets file
ESP_DEPLOY_PATH="../../../secrets/decrypted/esp_deploy/esp_deploy_data.txt"

# Run production script
python3 esp_provision_secrets.py $THING_SERIAL_PORT $THING_TYPE $THING_HW_VERSION $ESP_DEPLOY_PATH
