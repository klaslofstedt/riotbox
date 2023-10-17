#!/bin/bash

encrypt_directory() {
  local input_path="$1"
  local output_path="$2"

  # Create the output directory if it doesn't exist
  mkdir -p "$output_path"

  # Iterate through the items in the input directory
  for item in "$input_path"/*; do
    local item_name=$(basename "$item")
    local item_output_path="$output_path/$item_name"

    if [ -f "$item" ]; then
      # Encrypt the file and save it to the output directory
      openssl enc -aes-256-cbc -salt -in "$item" -out "$item_output_path.enc" -pbkdf2 -iter 100000 -pass pass:"$encryption_passphrase"
    elif [ -d "$item" ]; then
      # Recursively encrypt the subdirectory
      encrypt_directory "$item" "$item_output_path"
    fi
  done
}

# Read paths and password from environmental variabled
input_dir=$APP_SECRETS_DECRYPTED_PATH
output_dir=$APP_SECRETS_ENCRYPTED_PATH
encryption_passphrase=$SECRETS_PASSWORD


# Encrypt the input directory
encrypt_directory "$input_dir" "$output_dir"

echo "Encryption completed. Encrypted files can be found in the '/secrets/encrypted' directory."
