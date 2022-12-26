#!/bin/bash

set -xe

TOP_DIR=$(dirname $0)/..
CEF_BAT=$TOP_DIR/setup-cef.bat

VERSION=98.2.1+g29d6e22+chromium-98.0.4758.109
case $1 in
    stable)
	TARGET_VERSION=$(curl --silent https://cef-builds.spotifycdn.com/index.json | jq --raw-output '.windows64.versions[] | select(.channel == "stable").cef_version' | head -n 1)
	echo $TARGET_VERSION
	;;
    beta)
	TARGET_VERSION=$(curl --silent https://cef-builds.spotifycdn.com/index.json | jq --raw-output '.windows64.versions[] | select(.channel =="beta").cef_version' | head -n 1)
	echo $TARGET_VERSION
	sed -i'' -e s/windows32_minimal/windows32_beta_minimal/ $CEF_BAT
	;;
esac
sed -i'' -e s/$VERSION/$TARGET_VERSION/ $CEF_BAT
cat $CEF_BAT
