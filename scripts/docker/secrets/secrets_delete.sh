#!/bin/bash

# Define paths
ESP_AUTH_PATH="$APP_SECRETS_DECRYPTED_PATH/esp_auth/"
ESP_ENCRYPTION_PATH="$APP_SECRETS_DECRYPTED_PATH/esp_encryption/"
ESP_DEPLOY_PATH="$APP_SECRETS_DECRYPTED_PATH/esp_deploy/"
SECRETS_ENCRYPTED_PATH="$APP_SECRETS_ENCRYPTED_PATH"

# Define a function to safely remove files from a directory
safe_remove() {
    local dir=$1
    if [[ -d "$dir" ]]; then
        find "$dir" -mindepth 1 -delete
    else
        echo "Directory $dir does not exist!"
    fi
}

# Execute the functions with the defined paths
safe_remove "$ESP_AUTH_PATH"
safe_remove "$ESP_ENCRYPTION_PATH"
safe_remove "$ESP_DEPLOY_PATH"
safe_remove "$SECRETS_ENCRYPTED_PATH"
