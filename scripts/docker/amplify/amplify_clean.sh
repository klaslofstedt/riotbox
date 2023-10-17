#!/bin/bash

# The path to your app.json file
cd "$APP_AMPLIFY_PATH"

rm src/*
rm package-lock.json
rm -R node_modules
rm -R .expo
#rm -R amplify