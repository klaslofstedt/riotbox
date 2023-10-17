#!/bin/bash

CERTIFICATES_PATH=${APP_SECRETS_DECRYPTED_PATH}/esp_auth

# Fetch certificate
cert=$(curl https://www.amazontrust.com/repository/AmazonRootCA1.pem)

# Start of the .c file
echo "#include \"utilities/auth_aws_ota.h\"" > $CERTIFICATES_PATH/auth_aws_ota_root_ca.c
echo "" >> $CERTIFICATES_PATH/auth_aws_ota_root_ca.c
echo "const char auth_aws_ota_root_ca[] = " >> $CERTIFICATES_PATH/auth_aws_ota_root_ca.c

# Format the certificate content
IFS=$'\n' # Set Internal Field Separator to newline for read command
readarray -t lines <<< "$cert" # Read the certificate into an array
last_index=$(( ${#lines[@]} - 1 )) # Get the last index of the array

for index in "${!lines[@]}"; do
    line="${lines[$index]}"
    # If it's the last line of the certificate, add the semicolon
    if [[ "$index" -eq "$last_index" ]]; then
        echo "    \"$line\\n\";" >> $CERTIFICATES_PATH/auth_aws_ota_root_ca.c
    else
        echo "    \"$line\\n\" " >> $CERTIFICATES_PATH/auth_aws_ota_root_ca.c
    fi
done
