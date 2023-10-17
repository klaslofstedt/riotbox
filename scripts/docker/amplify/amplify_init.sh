#!/bin/bash

AMPLIFY_ENV_NAME="default" 
echo $PROFILE
echo $PROJECT_NAME
echo $AMPLIFY_ENV_NAME

cd $APP_AMPLIFY_PATH

AWSCLOUDFORMATIONCONFIG="{\
\"configLevel\":\"project\",\
\"useProfile\":true,\
\"profileName\":\"$PROFILE\"\
}"

AMPLIFY="{\
\"projectName\":\"$PROJECT_NAME\",\
\"envName\":\"$AMPLIFY_ENV_NAME\",\
\"defaultEditor\":\"None\"\
}"

REACTNATIVECONFIG="{\
\"SourceDir\":\"src\",\
\"DistributionDir\":\"build\",\
\"BuildCommand\":\"npm run-script build\",\
\"StartCommand\":\"npm run-script start\"\
}"

FRONTEND="{\
\"frontend\":\"javascript\",\
\"framework\":\"react-native\",\
\"config\":$REACTNATIVECONFIG\
}"

PROVIDERS="{\
\"awscloudformation\":$AWSCLOUDFORMATIONCONFIG\
}"

amplify init \
--amplify "$AMPLIFY" \
--frontend "$FRONTEND" \
--providers "$PROVIDERS" \
--yes \
--profile "$PROFILE"

sleep 5

amplify status

amplify pull