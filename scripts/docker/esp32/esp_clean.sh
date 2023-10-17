#!/bin/bash

rm -r "$APP_ESP32_PATH/build" || echo "Failed to remove $APP_ESP32_PATH/build"
rm -r "$APP_ESP32_PATH/managed_components/espressif__esp_encrypted_img" || echo "Failed to remove $APP_ESP32_PATH/managed_components/espressif__esp_encrypted_img"
rm "$APP_ESP32_PATH/sdkconfig" || echo "Failed to remove $APP_ESP32_PATH/sdkconfig"