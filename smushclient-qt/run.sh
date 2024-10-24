#!/bin/bash -e

CONFIG="Debug"
if [ "$1" = "--release" ]; then
  CONFIG="Release"
  shift
fi

DIR="$(dirname "$0")"
cd "$DIR"

BUILDDIR="$(pwd)/build/$CONFIG"

cmake -S . -B $BUILDDIR
cmake --build $BUILDDIR --config $CONFIG $@

mkdir -p ../environment
cd ../environment
mkdir -p lua
mkdir -p plugins
mkdir -p worlds

if [[ "$OSTYPE" == "darwin"* ]]; then
    if [[ "$CONFIG" == "Release" ]]; then
      cp -r "$BUILDDIR/SmushClient.app" .
      open "SmushClient.app"
    else
      "$BUILDDIR/SmushClient.app/Contents/MacOS/SmushClient"
    fi
fi
