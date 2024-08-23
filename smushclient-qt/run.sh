#!/bin/bash -e

cd "$(dirname "$0")"
RUN=1 ./build.sh $@
