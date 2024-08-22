#!/bin/bash -e

CONFIG="Debug"

if [ "$1" = "--release" ]; then
  CONFIG="Release"
fi

DIR="build/$CONFIG"

cmake -S . -B $DIR
cmake --build $DIR --config $CONFIG

if [ -z "$RUN" ]; then
  exit
fi

if [[ "$OSTYPE" == "darwin"* ]]; then
    $DIR/SmushClient.app/Contents/MacOS/SmushClient
fi
