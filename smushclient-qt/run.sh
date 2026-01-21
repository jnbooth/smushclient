#!/bin/bash -e

CONFIG="Debug"
if [ "$1" = "--release" ]; then
  CONFIG="Release"
  shift
fi

cd "$(dirname "$0")"
BUILDDIR="build/$CONFIG"

cmake -DCMAKE_BUILD_TYPE=$CONFIG -S . -B $BUILDDIR
# shellcheck disable=SC2068
cmake --build $BUILDDIR --config $CONFIG --target SmushClient $@

if [[ "$OSTYPE" == "darwin"* ]]; then
    if [[ "$CONFIG" == "Release" ]]; then
      open "$BUILDDIR/SmushClient.app"
    else
      "$BUILDDIR/SmushClient.app/Contents/MacOS/SmushClient"
    fi
fi
