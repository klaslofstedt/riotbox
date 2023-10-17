#!/bin/bash
set -e # exit when any command fails

trap 'last_command=$current_command; current_command=$BASH_COMMAND' DEBUG
trap 'echo "\"${last_command}\" script failed with exit code $?."' EXIT

./cdk/cdk_deploy.sh

trap - EXIT # Clear the trap