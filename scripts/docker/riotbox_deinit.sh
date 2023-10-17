#!/bin/bash

./aws/aws_delete_thing_certificates.sh
./secrets/secrets_delete.sh
./esp32/esp_clean.sh
./amplify/amplify_delete.sh
./amplify/amplify_deinit_appjson.sh
./amplify/amplify_clean.sh
./cdk/cdk_destroy.sh
./cdk/cdk_clean.sh

