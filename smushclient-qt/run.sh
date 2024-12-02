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
cmake --build $BUILDDIR --config $CONFIG --target SmushClient $@

if [[ "$OSTYPE" == "darwin"* ]]; then
    if [[ "$CONFIG" == "Release" ]]; then
      open "$BUILDDIR/SmushClient.app"
    else
      "$BUILDDIR/SmushClient.app/Contents/MacOS/SmushClient"
    fi
fi
