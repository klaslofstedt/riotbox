#!/bin/bash

# Due to an issue with CDK not being able to destroy a Hosted Zone
# without first deleting the CNAME records, we do that "manually".
# Issue here: https://github.com/aws/aws-cdk/issues/7063
#./aws_delete_cname_records.sh

# Due to not being able to generate Thing certificates from CDK we
# create them "manually" using aws cli, and hence we need to manually
# delete those certificates before destroying the CDK stack.
#./aws_delete_thing_certificates.sh

# Move to the directory containing the CDK stack and run destroy.
cd "$APP_CDK_PATH"
yes | cdk destroy --force --profile $PROFILE
