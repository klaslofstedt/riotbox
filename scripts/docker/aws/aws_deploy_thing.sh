#!/bin/bash

# Get the deploy data path
THING_DEPLOY_DATA_PATH="${APP_SECRETS_DECRYPTED_PATH}/esp_deploy/esp_deploy_data.txt"

echo $THING_DEPLOY_DATA_PATH
# Read data from the file
while IFS="=" read -r key value
do
  case "$key" in
    'DEPLOY_ID') id="$value" ;;
    'DEPLOY_TYPE') type="$value" ;;
    'DEPLOY_AES') aes="$value" ;;
    'DEPLOY_POP') pop="$value" ;;
  esac
done < "$THING_DEPLOY_DATA_PATH"

# Set the table name and new item properties
TABLE_NAME="${PROJECT_NAME}DynamoTableThings"
UTC_TIME_NOW=$(date -u +'%Y-%m-%dT%H:%M:%SZ')

ITEM='{
  "id": {"S": "'"$id"'"},
  "owner": {"S": "false"},
  "type": {"S": "'"$type"'"},
  "aes": {"S": "'"$aes"'"},
  "pop": {"S": "'"$pop"'"},
  "qr": {"S": "qr-code-url"},
  "deployed": {"S": "'"$UTC_TIME_NOW"'"},
  "provisioned": {"S": "false"},
  "synced": {"S": "false"},
  "updater": {"S": "false"},
  "value": {"S": "{\"mobile_value\": {\"readwrite\": {\"network\": \"offline\"},\"read\": {\"nickname\": \"'"$type"'\",\"sw_version\": \"0\"}}}"}
}'

# Update the table with the new item
aws dynamodb --profile $PROFILE put-item \
    --table-name "$TABLE_NAME" \
    --item "$ITEM"