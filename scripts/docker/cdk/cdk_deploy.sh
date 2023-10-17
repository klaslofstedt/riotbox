#!/bin/bash

# Change to the directory specified by APP_CDK_PATH
cd "$APP_CDK_PATH"
cdk synth --profile $PROFILE
cdk bootstrap --profile $PROFILE
yes | cdk deploy --force --require-approval never --profile $PROFILE --outputs-file "${APP_AMPLIFY_PATH}/src/cdk-exports.json"
