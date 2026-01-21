#!/bin/bash -e

CONFIG="Debug"

if [ "$1" = "--release" ]; then
  CONFIG="Release"
  shift
fi

cd "$(dirname "$0")"
BUILDDIR="build/$CONFIG"

rm -rf $BUILDDIR/SmushClient.app
cmake -DCMAKE_BUILD_TYPE=$CONFIG -S . -B $BUILDDIR
# shellcheck disable=SC2068
cmake --build $BUILDDIR --config $CONFIG --target update_translations --target SmushClient $@
