#!/bin/bash -e

CONFIG="Debug"
if [ "$1" = "--release" ]; then
  CONFIG="Release"
  shift
fi

DIR="$(dirname "$0")"
cd "$DIR"

BUILDDIR="$(pwd)/$DIR/build/$CONFIG"

cmake -S . -B $BUILDDIR
cmake --build $BUILDDIR --config $CONFIG $@

mkdir -p ../environment
cd ../environment
mkdir -p lua
mkdir -p plugins
mkdir -p worlds
if [[ "$OSTYPE" == "darwin"* ]]; then
    "$BUILDDIR/SmushClient.app/Contents/MacOS/SmushClient"
fi
