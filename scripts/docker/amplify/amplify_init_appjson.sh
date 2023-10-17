#!/bin/bash

APP_PROJECT_NAME=$PROJECT_NAME
APP_JSON_PATH="${APP_AMPLIFY_PATH}/app.json"

# Update the name and slug fields in the app.json file
jq ".expo.name = \"$APP_PROJECT_NAME\" | .expo.slug = \"$APP_PROJECT_NAME\"" "${APP_JSON_PATH}" > app_temp.json

mv app_temp.json "${APP_JSON_PATH}"


