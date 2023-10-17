#!/bin/bash

# Set output directory
output_dir="${APP_SECRETS_DECRYPTED_PATH}/esp_encryption"
output_file="${output_dir}/rsa_key.pem"

# Check if the destination directory exists, if not, create it
if [ ! -d "$output_dir" ]; then
  mkdir -p "$output_dir"
fi

# Generate RSA key directly into the correct folder
openssl genrsa -out "$output_file" 3072

# Print success message
echo "RSA key created at ${output_file}"