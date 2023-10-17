#!/bin/bash

AWS_ACCESS_KEY_ID=$AWS_ACCESS_KEY_ID
echo "AWS Access Key ID: $AWS_ACCESS_KEY_ID"

AWS_SECRET_ACCESS_KEY=$AWS_SECRET_ACCESS_KEY_ID
echo "AWS Secret Access Key: $AWS_SECRET_ACCESS_KEY"

REGION=$AWS_REGION
echo "Region: $REGION"

# Create the .aws directory inside /root if it doesn't exist
mkdir -p /root/.aws

# Create credentials file with the appropriate contents
cat > /root/.aws/credentials << EOL
[riotbox]
aws_access_key_id=$AWS_ACCESS_KEY_ID
aws_secret_access_key=$AWS_SECRET_ACCESS_KEY
EOL

# Create config file with the appropriate contents
cat > /root/.aws/config << EOL
[profile riotbox]
region=$REGION
EOL
