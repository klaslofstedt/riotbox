#!/bin/bash

APP_JSON_PATH="${APP_AMPLIFY_PATH}/app.json"

# Read the JSON file and update the specified keys
jq '(.expo.name = "") |
    (.expo.slug = "") |
    (.expo.ios.bundleIdentifier = "") |
    (.expo.extra.eas.projectId = "")' $APP_JSON_PATH > app_temp.json && mv app_temp.json $APP_JSON_PATH
