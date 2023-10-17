#!/bin/bash

# Recompile the firmware
rm -R "${APP_ESP32_PATH}/build"
"$APP_SCRIPTS_PATH/docker/esp32/esp_compile_thing.sh"

# Get the firmware path
OTA_FW_FILE_PATH=${APP_ESP32_PATH}/build/pre_encrypted_ota_secure.bin

# Extract the OTA_FW_FILE_NAME from the CMakeLists.txt
OTA_FW_FILE_NAME=$(grep "set(PROJECT_VER" ${APP_ESP32_PATH}/CMakeLists.txt | awk -F\" '{print $2}').bin

# Get AWS account ID
AWS_ACCOUNT_ID=$(aws sts get-caller-identity --query Account --output text --profile $PROFILE)
S3_BUCKET_NAME="$(echo ${PROJECT_NAME} | tr '[:upper:]' '[:lower:]')-bucket-thing-ota-fw-${AWS_ACCOUNT_ID}"

aws s3 cp $OTA_FW_FILE_PATH s3://$S3_BUCKET_NAME/$OTA_FW_FILE_NAME --profile $PROFILE
