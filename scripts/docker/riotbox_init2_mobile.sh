#!/bin/bash
set -e # exit when any command fails

trap 'last_command=$current_command; current_command=$BASH_COMMAND' DEBUG
trap 'echo "\"${last_command}\" script failed with exit code $?."' EXIT

./amplify/amplify_init.sh
./amplify/amplify_init_appjson.sh
./amplify/amplify_push.sh

trap - EXIT # Clear the trap