#!/bin/bash -e

CONFIG="Debug"

if [ "$1" = "--release" ]; then
  CONFIG="Release"
  shift
fi

BUILDDIR="build/$CONFIG"

cd "$(dirname "$0")"
cmake -S . -B $BUILDDIR
cmake --build $BUILDDIR --config $CONFIG --target update_translations --target all $@
