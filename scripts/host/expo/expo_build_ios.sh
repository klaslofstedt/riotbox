#!/bin/bash

# Navigate to the specified directory
cd ../../../app/mobile || exit 1

# Run the eas build command
eas build --profile development --platform ios