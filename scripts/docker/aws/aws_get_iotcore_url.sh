#!/bin/bash

# Define the path
FILE_PATH="${APP_SECRETS_DECRYPTED_PATH}/esp_auth/aws_mqtt_endpoint.h"

# Get the endpoint URL
ENDPOINT_URL=$(aws iot --profile $PROFILE describe-endpoint --endpoint-type iot:Data-ATS --query 'endpointAddress' --output text)

# Add the necessary prefix and port to the URL
MQTT_ENDPOINT="mqtts://${ENDPOINT_URL}:8883"

# Write to mqtt_endpoint.h
echo "#ifndef _AWS_MQTT_ENDPOINT_H_" > $FILE_PATH
echo "#define _AWS_MQTT_ENDPOINT_H_" >> $FILE_PATH
echo "" >> $FILE_PATH
echo "#define AWS_MQTT_BROKER_ENDPOINT_URL \"${MQTT_ENDPOINT}\"" >> $FILE_PATH
echo "" >> $FILE_PATH
echo "#endif /* _AWS_MQTT_ENDPOINT_H_ */" >> $FILE_PATH
