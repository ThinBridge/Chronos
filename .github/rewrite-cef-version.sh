#!/bin/bash

set -xe

TOP_DIR=$(dirname $0)/..
CEF_BAT=$TOP_DIR/setup-cef.bat

VERSION=119.4.3+gc76a3b9+chromium-119.0.6045.159
case $1 in
    stable)
	TARGET_VERSION=$(curl --silent https://cef-builds.spotifycdn.com/index.json | jq --raw-output '.windows32.versions[] | select(.channel == "stable").cef_version' | head -n 1)
	echo $TARGET_VERSION
	;;
    beta)
	TARGET_VERSION=$(curl --silent https://cef-builds.spotifycdn.com/index.json | jq --raw-output '.windows32.versions[] | select(.channel =="beta").cef_version' | head -n 1)
	echo $TARGET_VERSION
	sed -i'' -e s/windows32_minimal/windows32_beta_minimal/ $CEF_BAT
	;;
esac
sed -i'' -e s/$VERSION/$TARGET_VERSION/ $CEF_BAT
cat $CEF_BAT
