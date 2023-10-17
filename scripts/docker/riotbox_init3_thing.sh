#!/bin/bash
set -e # exit when any command fails

trap 'last_command=$current_command; current_command=$BASH_COMMAND' DEBUG
trap 'echo "\"${last_command}\" script failed with exit code $?."' EXIT

./aws/aws_create_thing_certificates.sh
./aws/aws_create_ca_root_certificate.sh
./aws/aws_get_iotcore_url.sh
./esp32/esp_create_rsa_key.sh
./aws/aws_upload_ota_to_s3.sh

trap - EXIT # Clear the trap