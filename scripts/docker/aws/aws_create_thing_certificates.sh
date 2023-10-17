#!/bin/bash

# Set the AWS region
THING_NAME=${PROJECT_NAME}IoTCoreThing
TIMESTAMP=$(date +%s) # Get current timestamp
POLICY_NAME=${PROJECT_NAME}IoTCoreThingPolicy-$TIMESTAMP
CERTIFICATES_PATH=${APP_SECRETS_DECRYPTED_PATH}/esp_auth
SECRETS_MANAGER_NAME=${PROJECT_NAME}SecretsManagerIoTCoreThingCerts
CURRENT_PATH=$(pwd)

# Get AWS account ID
AWS_ACCOUNT_ID=$(aws sts get-caller-identity --query Account --output text --profile $PROFILE)
S3_BUCKET_NAME="$(echo ${PROJECT_NAME} | tr '[:upper:]' '[:lower:]')-bucket-thing-secrets-${AWS_ACCOUNT_ID}"

# Create the client certificate and private key
aws iot --profile $PROFILE create-keys-and-certificate --set-as-active > cert_output.json

# Extract the certificate ARN, certificate PEM, and private key from the output
CERTIFICATE_ARN=$(jq -r '.certificateArn' cert_output.json)
CERTIFICATE_PEM=$(jq -r '.certificatePem' cert_output.json)
PRIVATE_KEY=$(jq -r '.keyPair.PrivateKey' cert_output.json)

# Save the certificate PEM and private key to C files
# Start of the .c files
echo "#include \"utilities/auth_aws_ota.h\"" > $CERTIFICATES_PATH/auth_aws_ota_thing_cert.c
echo "" >> $CERTIFICATES_PATH/auth_aws_ota_thing_cert.c
echo "const char auth_aws_ota_thing_cert[] = " >> $CERTIFICATES_PATH/auth_aws_ota_thing_cert.c

echo "#include \"utilities/auth_aws_ota.h\"" > $CERTIFICATES_PATH/auth_aws_ota_thing_key.c
echo "" >> $CERTIFICATES_PATH/auth_aws_ota_thing_key.c
echo "const char auth_aws_ota_thing_key[] = " >> $CERTIFICATES_PATH/auth_aws_ota_thing_key.c

# Format and save the certificate and key content
printf "%s" "$CERTIFICATE_PEM" | while IFS= read -r line
do
    echo "    \"$line\\n\" " >> $CERTIFICATES_PATH/auth_aws_ota_thing_cert.c
done
echo "    \"-----END CERTIFICATE-----\\n\";" >> $CERTIFICATES_PATH/auth_aws_ota_thing_cert.c

printf "%s" "$PRIVATE_KEY" | while IFS= read -r line
do
    echo "    \"$line\\n\" " >> $CERTIFICATES_PATH/auth_aws_ota_thing_key.c
done
echo "    \"-----END RSA PRIVATE KEY-----\\n\";" >> $CERTIFICATES_PATH/auth_aws_ota_thing_key.c

# Save the certificate and private key as PEM files in the current directory
echo "$CERTIFICATE_PEM" > $CURRENT_PATH/auth_aws_ota_thing_cert.pem
echo "$PRIVATE_KEY" > $CURRENT_PATH/auth_aws_ota_thing_key.pem

# Upload the certificate and private key to S3 bucket
aws s3 cp $CURRENT_PATH/auth_aws_ota_thing_cert.pem s3://$S3_BUCKET_NAME/public/auth_aws_ota_thing_cert.pem --profile $PROFILE
aws s3 cp $CURRENT_PATH/auth_aws_ota_thing_key.pem s3://$S3_BUCKET_NAME/public/auth_aws_ota_thing_key.pem --profile $PROFILE

# Remove the PEM files after uploading
rm $CURRENT_PATH/auth_aws_ota_thing_cert.pem
rm $CURRENT_PATH/auth_aws_ota_thing_key.pem

# Attach the certificate to your IoT Thing
aws iot --profile $PROFILE attach-thing-principal --thing-name $THING_NAME --principal $CERTIFICATE_ARN


# Create a policy allowing the certificate to access the IoT Core
POLICY_DOCUMENT='{
  "Version": "2012-10-17",
  "Statement": [
    {
      "Effect": "Allow",
      "Action": "iot:*",
      "Resource": "*"
    }
  ]
}'

aws iot --profile $PROFILE create-policy --policy-name $POLICY_NAME --policy-document "$POLICY_DOCUMENT"

# Attach the policy to the certificate
aws iot --profile $PROFILE attach-policy --policy-name $POLICY_NAME --target $CERTIFICATE_ARN

rm cert_output.json