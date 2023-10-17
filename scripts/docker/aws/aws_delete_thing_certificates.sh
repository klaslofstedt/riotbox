#!/bin/bash

THING_NAME=${PROJECT_NAME}IoTCoreThing

# Get the ARNs of all principals attached to the specific thing
PRINCIPAL_ARNS=$(aws iot list-thing-principals --thing-name $THING_NAME --profile $PROFILE --query 'principals[*]' --output text)

if [ "$PRINCIPAL_ARNS" != "None" ]; then
    for PRINCIPAL_ARN in $PRINCIPAL_ARNS
    do
        CERTIFICATE_ID=$(basename $PRINCIPAL_ARN)
        
        # Detach all associated policies and delete them
        POLICIES=$(aws iot list-principal-policies --principal $PRINCIPAL_ARN --profile $PROFILE --query 'policies[*].policyName' --output text)
        for POLICY_NAME in $POLICIES
        do
            if [ "$POLICY_NAME" != "None" ]; then
                echo "Detaching and Deleting policy $POLICY_NAME from certificate $CERTIFICATE_ID"
                aws iot detach-policy --policy-name $POLICY_NAME --target $PRINCIPAL_ARN --profile $PROFILE
                aws iot delete-policy --policy-name $POLICY_NAME --profile $PROFILE
            fi
        done
        
        # Detach the certificate from the thing and delete the certificate
        echo "Detaching and Deleting certificate $CERTIFICATE_ID from thing $THING_NAME"
        aws iot detach-thing-principal --thing-name $THING_NAME --principal $PRINCIPAL_ARN --profile $PROFILE
        aws iot update-certificate --certificate-id $CERTIFICATE_ID --new-status INACTIVE --profile $PROFILE
        aws iot delete-certificate --certificate-id $CERTIFICATE_ID --profile $PROFILE
    done
else
    echo "No principal attached to the thing $THING_NAME"
fi


CERTIFICATES_PATH=${APP_SECRETS_DECRYPTED_PATH}/esp_auth
# Get AWS account ID
AWS_ACCOUNT_ID=$(aws sts get-caller-identity --query Account --output text --profile $PROFILE)
S3_BUCKET_NAME="$(echo ${PROJECT_NAME} | tr '[:upper:]' '[:lower:]')-bucket-thing-secrets-${AWS_ACCOUNT_ID}"

# Delete certificates from S3
aws s3 rm s3://$S3_BUCKET_NAME/public/auth_aws_ota_thing_cert.pem --profile $PROFILE
aws s3 rm s3://$S3_BUCKET_NAME/public/auth_aws_ota_thing_key.pem --profile $PROFILE

# Remove the created files
rm $CERTIFICATES_PATH/auth_aws_ota_thing_cert.c
rm $CERTIFICATES_PATH/auth_aws_ota_thing_key.c

# Output the removed files
echo "Removed: $CERTIFICATES_PATH/auth_aws_ota_thing_cert.c"
echo "Removed: $CERTIFICATES_PATH/auth_aws_ota_thing_key.c"
