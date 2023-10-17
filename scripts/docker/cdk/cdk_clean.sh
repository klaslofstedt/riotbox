#!/bin/bash

# The path to your app.json file
cd "$APP_CDK_PATH"

rm package-lock.json
rm -R node_modules
rm -R cdk.out

cd "${APP_CDK_PATH}/lambdas/api-things"
rm -R node_modules
rm package-lock.json

cd "${APP_CDK_PATH}/lambdas/iotcore-cloud-to-thing"
rm -R node_modules
rm package-lock.json

cd "${APP_CDK_PATH}/lambdas/iotcore-thing-to-cloud"
rm -R node_modules
rm package-lock.json
