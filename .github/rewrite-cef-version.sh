#!/bin/bash

set -xe

TOP_DIR=$(dirname $0)/..
CEF_BAT=$TOP_DIR/setup-cef.bat

VERSION=131.2.4+gb7543e4+chromium-131.0.6778.70
case $1 in
    stable)
	TARGET_VERSION=$(curl --silent https://cef-builds.spotifycdn.com/index.json | jq '.windows32.versions[] | select(.channel =="stable")' | jq -s --raw-output 'max_by(.chromium_version | split(".") | map(tonumber)) | .cef_version')
	echo $TARGET_VERSION
	;;
    beta)
	TARGET_VERSION=$(curl --silent https://cef-builds.spotifycdn.com/index.json | jq '.windows32.versions[] | select(.channel =="beta")' | jq -s --raw-output 'max_by(.chromium_version | split(".") | map(tonumber)) | .cef_version')
	echo $TARGET_VERSION
	sed -i'' -e s/windows32_minimal/windows32_beta_minimal/ $CEF_BAT
	;;
esac
sed -i'' -e s/$VERSION/$TARGET_VERSION/ $CEF_BAT
cat $CEF_BAT
